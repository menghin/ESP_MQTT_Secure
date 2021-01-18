/* mqtt_sensor_data
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mqtt_sensor_data.h>

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
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"

#define BUFFER_SIZE 400

#define SENSOR_DATA_START_ADDR ((uint32_t *)0x50000000)
static RTC_DATA_ATTR struct sensor_data *sensor_data_buffer = (struct sensor_data *)SENSOR_DATA_START_ADDR;

uint32_t mqtt_sensor_data_count(void)
{
    return ((uint32_t)sensor_data_buffer - (uint32_t)SENSOR_DATA_START_ADDR) / sizeof(struct sensor_data);
}

void mqtt_sensor_data_push(struct sensor_data *item)
{
    if (mqtt_sensor_data_count() < BUFFER_SIZE)
    {
        memcpy((void *)sensor_data_buffer, (void *)item, sizeof(struct sensor_data));
        sensor_data_buffer++;
    }
}

void mqtt_sensor_data_pop(struct sensor_data *item)
{
    if (mqtt_sensor_data_count() > 0)
    {
        memcpy((void *)item, (void *)(sensor_data_buffer-1), sizeof(struct sensor_data));
        sensor_data_buffer--;
    }
}