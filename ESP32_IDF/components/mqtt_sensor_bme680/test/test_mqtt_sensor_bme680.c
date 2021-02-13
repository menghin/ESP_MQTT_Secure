/* test_mqtt_sensor_bme680.c: Implementation of a mqtt_sensor_bme680 component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <limits.h>
#include "unity.h"
#include <sys/time.h>
#include <mqtt_sensor_bme680.h>

TEST_CASE("mqtt_sensor_bme680_init()", "[mqtt_sensor_bme680]")
{
    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_bme680_init());

    i2c_driver_delete(I2C_BUS);
}

TEST_CASE("mqtt_sensor_bme680_get_results_blocking() get results", "[mqtt_sensor_bme680]")
{
    time_t now;
    time(&now);
    vTaskDelay(200);

    TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_bme680_init());

    for (int i = 0; i < 25; i++)
    {
        float paintValue = 121212.121212;
        struct sensor_data results;
        results.gasResistance = paintValue;
        results.humidity = paintValue;
        results.pressure = paintValue;
        results.temperature = paintValue;
        results.timestamp = now;
        TEST_ASSERT_EQUAL(ESP_OK, mqtt_sensor_bme680_get_results_blocking(&results));
        TEST_ASSERT_NOT_EQUAL(paintValue, results.gasResistance);
        TEST_ASSERT_NOT_EQUAL(paintValue, results.humidity);
        TEST_ASSERT_LESS_OR_EQUAL(100.0, results.humidity);
        TEST_ASSERT_NOT_EQUAL(paintValue, results.pressure);
        TEST_ASSERT_LESS_OR_EQUAL(2000.0, results.pressure);
        TEST_ASSERT_NOT_EQUAL(paintValue, results.temperature);
        TEST_ASSERT_LESS_OR_EQUAL(60.0, results.temperature);
        TEST_ASSERT_GREATER_THAN(now, results.timestamp);
    }

    i2c_driver_delete(I2C_BUS);
}

TEST_CASE("mqtt_sensor_bme680_get_results_blocking() i2c not initalized", "[mqtt_sensor_bme680]")
{
    time_t now;
    time(&now);
    vTaskDelay(200);

    for (int i = 0; i < 25; i++)
    {
        float paintValue = 121212.121212;
        struct sensor_data results;
        results.gasResistance = paintValue;
        results.humidity = paintValue;
        results.pressure = paintValue;
        results.temperature = paintValue;
        results.timestamp = now;
        TEST_ASSERT_EQUAL(ESP_FAIL, mqtt_sensor_bme680_get_results_blocking(&results));
        TEST_ASSERT_EQUAL(paintValue, results.gasResistance);
        TEST_ASSERT_EQUAL(paintValue, results.humidity);
        TEST_ASSERT_EQUAL(paintValue, results.pressure);
        TEST_ASSERT_EQUAL(paintValue, results.temperature);
        TEST_ASSERT_EQUAL(now, results.timestamp);
    }
}
