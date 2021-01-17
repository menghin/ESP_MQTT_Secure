/* Main of MQTT client sending the sensor data via WIFI and then entering deep_sleep again.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp32/ulp.h"
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "soc/sens_periph.h"
#include "soc/rtc.h"

#define WAKEUP_TIME_SEC 5  //!< Time how long the device goes to deep sleep

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
#if CONFIG_IDF_TARGET_ESP32
    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#endif

    // Main process


    // Code executed when leaving app_main
    gettimeofday(&leave_app_main_time, NULL);
    app_main_duration_ms = (leave_app_main_time.tv_sec - enter_app_main_time.tv_sec) * 1000 + (leave_app_main_time.tv_usec - enter_app_main_time.tv_usec) / 1000;
    ESP_LOGI("app_main", "Entering deep sleep again after %d ms.", app_main_duration_ms);
    esp_deep_sleep_start();
} 
