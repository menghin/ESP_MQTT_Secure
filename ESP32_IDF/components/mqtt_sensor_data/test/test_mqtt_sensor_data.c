/* test_mqtt_sensor_data.c: Implementation of a mqtt_sensor_data component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <limits.h>
#include "unity.h"
#include "mqtt_sensor_data.h"

TEST_CASE("mqtt_sensor_data_count is intially 0", "[mqtt_sensor_data]")
{
    TEST_ASSERT_EQUAL(0, mqtt_sensor_data_count());
}

TEST_CASE("mqtt_sensor_data_count increases until max", "[mqtt_sensor_data]")
{
    for (uint32_t i = 0; i < MQTT_SENSOR_DATA_BUFFER_SIZE; i++)
    {
        struct sensor_data item;
        mqtt_sensor_data_push(&item);

        TEST_ASSERT_EQUAL(i + 1, mqtt_sensor_data_count());
    }

    for (uint32_t i = MQTT_SENSOR_DATA_BUFFER_SIZE; i > 0; i--)
    {
        mqtt_sensor_data_drop();
        TEST_ASSERT_EQUAL(i - 1, mqtt_sensor_data_count());
    }
}

TEST_CASE("mqtt_sensor_data_count increases stays max when overflow", "[mqtt_sensor_data]")
{
    struct sensor_data item;
    for (uint32_t i = 0; i < MQTT_SENSOR_DATA_BUFFER_SIZE + 1; i++)
    {
        mqtt_sensor_data_push(&item);
    }

    TEST_ASSERT_EQUAL(MQTT_SENSOR_DATA_BUFFER_SIZE, mqtt_sensor_data_count());
    mqtt_sensor_data_push(&item);
    TEST_ASSERT_EQUAL(MQTT_SENSOR_DATA_BUFFER_SIZE, mqtt_sensor_data_count());

    for (uint32_t i = MQTT_SENSOR_DATA_BUFFER_SIZE; i > 0; i--)
    {
        mqtt_sensor_data_drop();
    }
}

TEST_CASE("mqtt_sensor_data_count increases stays 0 when underflow", "[mqtt_sensor_data]")
{
    mqtt_sensor_data_drop();
    TEST_ASSERT_EQUAL(0, mqtt_sensor_data_count());
    mqtt_sensor_data_drop();
    TEST_ASSERT_EQUAL(0, mqtt_sensor_data_count());
}

TEST_CASE("mqtt_sensor_data_count stores correct data", "[mqtt_sensor_data]")
{
    struct sensor_data item;
    for (int i = 0; i < MQTT_SENSOR_DATA_BUFFER_SIZE; i++)
    {
        item.gasResistance = i;
        item.humidity = i + 1;
        item.pressure = i + 2;
        item.temperature = i + 3;
        item.timestamp = i + 4;
        mqtt_sensor_data_push(&item);
    }

    for (int i = MQTT_SENSOR_DATA_BUFFER_SIZE-1; i >= 0; i--)
    {
        mqtt_sensor_data_get_last(&item);
        TEST_ASSERT_EQUAL(i, item.gasResistance);
        TEST_ASSERT_EQUAL(i+1, item.humidity);
        TEST_ASSERT_EQUAL(i+2, item.pressure);
        TEST_ASSERT_EQUAL(i+3, item.temperature);
        TEST_ASSERT_EQUAL(i+4, item.timestamp);
        mqtt_sensor_data_drop();
    }
}
