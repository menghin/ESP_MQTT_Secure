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

#define USE_SECRET_LOCAL

#ifdef USE_SECRET_LOCAL
#include "mqtt_sensor_wifi_secret_local.h"
#include "mqtt_sensor_mqtt_secret_local.h"
#else
/* Used WIFI SSID*/
#define ESP_WIFI_SSID "WIFI_SSID"
/* Used WIFI password*/
#define ESP_WIFI_PASS "WIFI_PASS"
/* Used WIFI Channel*/
#define ESP_WIFI_CHANNEL 0

#define LOCATION "location"
#define HOSTNAME LOCATION "hostname"

const char *MQTT_URI = "URI";
const char *MQTT_USER = "user";    // leave blank if no credentials used
const char *MQTT_PASS = "pasword"; // leave blank if no credentials used

#endif

#define WAKEUP_TIME_SEC 120 //!< Time how long the device goes to deep sleep

static const char *TAG = "main";

#define ONE_TIME_INIT_ADDR ((uint32_t *)0x50000010)
#define ONE_TIME_INIT_MAGIC_WORD 0xBABABABA
static uint32_t *one_time_init = (uint32_t *)ONE_TIME_INIT_ADDR;
#define ALIVE_COUNT_ADDR ((uint32_t *)0x50000014)
static uint32_t *alive_count = (uint32_t *)ALIVE_COUNT_ADDR;

void app_main(void)
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL};

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic_result = LOCATION "/" HOSTNAME "/out/result";
    char *mqtt_pub_topic_message = LOCATION "/" HOSTNAME "/out/message";

    // Code executed when entering app_main
    struct timeval enter_app_main_time;
    struct timeval leave_app_main_time;
    int app_main_duration_ms;
    esp_log_level_set("*", ESP_LOG_WARN);
    gettimeofday(&enter_app_main_time, NULL);
    const int wakeup_time_sec = WAKEUP_TIME_SEC;
    ESP_LOGI(TAG, "Enabling timer wakeup, %d s", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);

    if (*one_time_init != ONE_TIME_INIT_MAGIC_WORD)
    {
        *one_time_init = ONE_TIME_INIT_MAGIC_WORD;
        *alive_count = 0;
    }
    else
    {
        (*alive_count)++;
    }

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

    if (mqtt_sensor_bme680_init() == ESP_OK)
    {
        if (mqtt_sensor_bme680_get_results_blocking(&results) == ESP_OK)
        {
            ESP_LOGI(TAG, "%.3f BME680 Sensor (push): %.2f °C, %.2f %%, %.2f hPa, %.2f Ohm",
                     (double)sdk_system_get_time() * 1e-3,
                     results.temperature, results.humidity,
                     results.pressure, results.gasResistance);
            mqtt_sensor_data_push(&results);

            if (mqtt_sensor_wifi_connect_to_sta(wifi_config) == ESP_OK)
            {
                mqtt_sensor_sntp_sync();

                if (mqtt_sensor_mqtt_connect(mqtt_cfg, mqtt_pub_topic_result, mqtt_pub_topic_message) == ESP_OK)
                {
                    uint32_t current_buffer_index = mqtt_sensor_data_count();
                    ESP_LOGI(TAG, "current_buffer_index %d.", current_buffer_index);

                    for (uint32_t i = 0; i < current_buffer_index; i++)
                    {
                        mqtt_sensor_data_get_last(&results);
                        if (mqtt_sensor_mqtt_publish_result(&results) == ESP_OK)
                        {
                            mqtt_sensor_data_drop();
                        }
                        else
                        {
                            ESP_LOGI(TAG, "mqtt_sensor_mqtt_publish failed - will retry next iteration.");
                            break;
                        }

                        char value_str[32] = {0};
                        sprintf(value_str, "alive_count = %d", *alive_count);
                        mqtt_sensor_mqtt_publish_message(value_str);
                    }
                }
                mqtt_sensor_mqtt_disconnect();
            }
            mqtt_sensor_wifi_disconnect_to_sta();
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
