/* mqtt_sensor_bme680
*/

#include <string.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <mqtt_sensor_bme680.h>

#define ONE_TIME_INIT_ADDR ((uint32_t *)0x50000000)
#define ONE_TIME_INIT_MAGIC_WORD 0xBABABABA
static uint32_t *one_time_init = (uint32_t *)ONE_TIME_INIT_ADDR;

#define SENSOR_INSTANCE_START_ADDR ((uint32_t *)0x50000004)
static bme680_sensor_t *sensor = (bme680_sensor_t *)SENSOR_INSTANCE_START_ADDR;

static const char *TAG = "mqtt_sensor_bme680";

static esp_err_t mqtt_sensor_bme680_init(void)
{
    esp_err_t status = ESP_FAIL;

    // Init all I2C bus interfaces at which BME680 sensors are connected
    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);

    if (*one_time_init != ONE_TIME_INIT_MAGIC_WORD)
    {
        bme680_sensor_t *new_sensor_instance;

        // init the sensor with slave address BME680_I2C_ADDRESS_2 connected to I2C_BUS.
        new_sensor_instance = bme680_init_sensor(I2C_BUS, BME680_I2C_ADDRESS_2, 0);

        // Store instance in RTC area
        memcpy(sensor, new_sensor_instance, sizeof(bme680_sensor_t));

        if (sensor)
        {
            /** -- SENSOR CONFIGURATION PART (optional) --- */

            // Changes the oversampling rates to 4x oversampling for temperature
            // and 2x oversampling for humidity. Pressure measurement is skipped.
            bme680_set_oversampling_rates(sensor, osr_8x, osr_2x, osr_4x);

            // Change the IIR filter size for temperature and pressure to 7.
            bme680_set_filter_size(sensor, iir_size_7);

            // Change the heater profile 0 to 200 degree Celcius for 100 ms.
            bme680_set_heater_profile(sensor, 0, 320, 150);
            bme680_use_heater_profile(sensor, 0);

            // Set ambient temperature to 10 degree Celsius
            bme680_set_ambient_temperature(sensor, 10);

            status = ESP_OK;

            *one_time_init = ONE_TIME_INIT_MAGIC_WORD;
        }
        else
        {
            ESP_LOGE(TAG, "Could not initialize BME680 sensor");
        }
    }
    else
    {
        status = ESP_OK;
    }

    return status;
}

esp_err_t mqtt_sensor_bme680_get_results_blocking(struct sensor_data *results)
{
    esp_err_t status = ESP_FAIL;
    bme680_values_float_t bme680_result;

    if (mqtt_sensor_bme680_init() == ESP_OK)
    {
        // trigger the sensor to start one TPHG measurement cycle
        if (bme680_force_measurement(sensor))
        {
            // as long as sensor configuration isn't changed, duration is constant
            uint32_t duration = bme680_get_measurement_duration(sensor);

            // passive waiting until measurement results are available
            vTaskDelay(duration);

            // get the results and do something with them
            if (bme680_get_results_float(sensor, &bme680_result))
            {
                results->temperature = bme680_result.temperature;
                results->humidity = bme680_result.humidity;
                results->pressure = bme680_result.pressure;
                results->gasResistance = bme680_result.gas_resistance;
                results->timestamp = 1611512746;

                status = ESP_OK;
            }
        }
        else
        {
            ESP_LOGE(TAG, "Could not get sensor data from BME680 sensor");
        }
    }

    return status;
}
