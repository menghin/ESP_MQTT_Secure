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

#define WAKEUP_TIME_SEC 5 //!< Time how long the device goes to deep sleep

void app_main(void)
{
    // Code executed when entering app_main
    struct timeval enter_app_main_time;
    struct timeval leave_app_main_time;
    int app_main_duration_ms;
    esp_log_level_set("*", ESP_LOG_INFO);
    gettimeofday(&enter_app_main_time, NULL);
    const int wakeup_time_sec = WAKEUP_TIME_SEC;
    ESP_LOGI("app_main", "Enabling timer wakeup, %d s", wakeup_time_sec);
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
    if (mqtt_sensor_wifi_connect_to_sta() == ESP_OK)
    {
        // Blink once
        gpio_pad_select_gpio(GPIO_NUM_5);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_5, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_5, 0);

        uint32_t current_buffer_index = mqtt_sensor_data_count();

        for (int32_t i = 0; i < 10; i++)
        {
            struct sensor_data item;
            item.gasResistance = current_buffer_index;
            mqtt_sensor_data_push(&item);
        }

        ESP_LOGI("app_main", "current_buffer_index %d.", current_buffer_index);
        struct sensor_data item;
        mqtt_sensor_data_pop(&item);
        ESP_LOGI("app_main", "one poped item %f.", item.gasResistance);
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
    ESP_LOGI("app_main", "Entering deep sleep again after %d ms.", app_main_duration_ms);
    esp_deep_sleep_start();
}
