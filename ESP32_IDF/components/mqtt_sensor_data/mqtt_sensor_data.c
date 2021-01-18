/* mqtt_sensor_data
*/

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <mqtt_sensor_data.h>

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
        memcpy((void *)item, (void *)(sensor_data_buffer - 1), sizeof(struct sensor_data));
        sensor_data_buffer--;
    }
}
