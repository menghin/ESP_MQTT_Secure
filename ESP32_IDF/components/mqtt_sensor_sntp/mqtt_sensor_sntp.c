/* mqtt_sensor_data
*/

#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include "esp_sntp.h"
#include <mqtt_sensor_sntp.h>

#define ONE_TIME_INIT_ADDR ((uint32_t *)0x50000004)
#define ONE_TIME_INIT_MAGIC_WORD 0xBABABABA
static uint32_t *one_time_init = (uint32_t *)ONE_TIME_INIT_ADDR;
#define SYNC_COUNTER_ADDR ((uint32_t *)0x50000008)
#define SYNC_AFTER_N_TIMES 100
static uint32_t *sync_counter = (uint32_t *)SYNC_COUNTER_ADDR;

// Address 0x50000000 is reserved for one time init of BME680
// Address 0x5000000C is reserved for one time init of DATA

static const char *TAG = "mqtt_sensor_sntp";

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void mqtt_sensor_sntp_sync(void)
{
    if (*one_time_init != ONE_TIME_INIT_MAGIC_WORD)
    {
        *one_time_init = ONE_TIME_INIT_MAGIC_WORD;
        *sync_counter = 0;
    }

    if (*sync_counter >= SYNC_AFTER_N_TIMES)
    {
        (*sync_counter) = 0;
    }

    if (*sync_counter == 0)
    {
        int retry = 0;
        const int retry_count = 10;

        initialize_sntp();

        // wait for time to be set
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
        {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }

    (*sync_counter)++;
}
