/* Main of MQTT client sending the sensor data via WIFI and then entering deep_sleep again.
*/

#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <driver/rtc_io.h>
#include <nvs_flash.h>

#include <mqtt_sensor_wifi.h>
#include <mqtt_sensor_data.h>
#include <mqtt_sensor_bme680.h>
#include <mqtt_sensor_mqtt.h>
#include <mqtt_sensor_sntp.h>

#define WAKEUP_TIME_SEC 120 //!< Time how long the device goes to deep sleep

static const char *TAG = "main";

void app_main(void)
{
    // Code executed when entering app_main
    struct timeval enter_app_main_time;
    struct timeval leave_app_main_time;
    int app_main_duration_ms;
    esp_log_level_set("*", ESP_LOG_INFO);
    gettimeofday(&enter_app_main_time, NULL);
    const int wakeup_time_sec = WAKEUP_TIME_SEC;
    ESP_LOGI(TAG, "Enabling timer wakeup, %d s", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Main process
    struct sensor_data results;

    if (mqtt_sensor_bme680_get_results_blocking(&results) == ESP_OK)
    {
        ESP_LOGI(TAG, "%.3f BME680 Sensor (push): %.2f °C, %.2f %%, %.2f hPa, %.2f Ohm",
                 (double)sdk_system_get_time() * 1e-3,
                 results.temperature, results.humidity,
                 results.pressure, results.gasResistance);
        mqtt_sensor_data_push(&results);

        if (mqtt_sensor_wifi_connect_to_sta() == ESP_OK)
        {
            mqtt_sensor_sntp_sync();

            mqtt_sensor_mqtt_connect();

            uint32_t current_buffer_index = mqtt_sensor_data_count();
            ESP_LOGI(TAG, "current_buffer_index %d.", current_buffer_index);

            for (uint32_t i = 0; i < current_buffer_index; i++)
            {
                mqtt_sensor_data_pop(&results);
                mqtt_sensor_mqtt_publish(&results);
            }
        }
    }

// Code executed when leaving app_main
#if CONFIG_IDF_TARGET_ESP32
    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#endif
    gettimeofday(&leave_app_main_time, NULL);
    app_main_duration_ms = (leave_app_main_time.tv_sec - enter_app_main_time.tv_sec) * 1000 + (leave_app_main_time.tv_usec - enter_app_main_time.tv_usec) / 1000;
    ESP_LOGW(TAG, "Entering deep sleep again after %d ms.", app_main_duration_ms);
    esp_deep_sleep_start();
}
