/* Main of MQTT client sending the sensor data via WIFI and then entering deep_sleep again.
*/

#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <driver/rtc_io.h>
#include <nvs_flash.h>

#include "driver/gpio.h"
#include "driver/adc.h"

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

#define NO_OF_SAMPLES 64                            //Multisampling
static const adc_channel_t channel = ADC_CHANNEL_6; //GPIO34 if ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;

static float readBatteryLevel()
{

    float batteryLevel = 0;
    uint32_t adc_reading = 0;

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);

    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
        adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    adc_reading /= NO_OF_SAMPLES;
    batteryLevel = adc_reading * 100 / 4095;

    ESP_LOGI(TAG, "Battery level = %f Percent", batteryLevel);

    return batteryLevel;
}

void app_main(void)
{
    mqtt_sensor_wifi_config_t wifi_config = {
        .ssid = ESP_WIFI_SSID,
        .password = ESP_WIFI_PASS,
        .channel = ESP_WIFI_CHANNEL};

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URI,
        .username = MQTT_USER,
        .password = MQTT_PASS};

    char *mqtt_pub_topic = LOCATION "/" HOSTNAME "/out";

    // Code executed when entering app_main
    struct timeval enter_app_main_time;
    struct timeval leave_app_main_time;
    int app_main_duration_ms;
    esp_log_level_set("*", ESP_LOG_WARN);
    gettimeofday(&enter_app_main_time, NULL);
    const int wakeup_time_sec = WAKEUP_TIME_SEC;
    ESP_LOGI(TAG, "Enabling timer wakeup, %d s", wakeup_time_sec);

    // Get the batteryLevel
    float batteryLevel = readBatteryLevel();

    if (batteryLevel >= 55)
    {
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

        if (mqtt_sensor_bme680_init() == ESP_OK)
        {
            if (mqtt_sensor_bme680_get_results_blocking(&results) == ESP_OK)
            {
                ESP_LOGI(TAG, "%.3f BME680 Sensor (push): %.2f °C, %.2f %%, %.2f hPa, %.2f Ohm",
                         (double)sdk_system_get_time() * 1e-3,
                         results.temperature, results.humidity,
                         results.pressure, results.gasResistance);
                mqtt_sensor_data_push(&results);

                if (mqtt_sensor_wifi_connect_to_sta(&wifi_config) == ESP_OK)
                {
                    mqtt_sensor_sntp_sync();

                    if (mqtt_sensor_mqtt_connect(&mqtt_cfg, mqtt_pub_topic) == ESP_OK)
                    {
                        uint32_t current_buffer_index = mqtt_sensor_data_count();
                        ESP_LOGI(TAG, "current_buffer_index %d.", current_buffer_index);

                        // Transmit only 50 results to avoid big delays in acquiring the sensor data
                        if (current_buffer_index > 50)
                        {
                            current_buffer_index = 50;
                        }

                        for (uint32_t i = 0; i < current_buffer_index; i++)
                        {
                            mqtt_sensor_data_get_last(&results);
                            if (mqtt_sensor_mqtt_publish_result(&results, batteryLevel) == ESP_OK)
                            {
                                mqtt_sensor_data_drop();
                            }
                            else
                            {
                                ESP_LOGI(TAG, "mqtt_sensor_mqtt_publish failed - will retry next iteration.");
                            }
                        }
                    }
                    mqtt_sensor_mqtt_disconnect();
                }
                mqtt_sensor_wifi_disconnect_to_sta();
            }
        }
    }
    else
    {
        // if teh battery level is low it will goto sleep for a
        // longer time to save battery
        esp_sleep_enable_timer_wakeup(wakeup_time_sec * 5 * 1000000);
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
