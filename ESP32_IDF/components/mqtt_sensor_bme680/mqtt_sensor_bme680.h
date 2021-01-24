/* Functions used to access the BME680
*/

#ifndef __MQTT_SENSOR_BME680_H_
#define __MQTT_SENSOR_BME680_H_

// I2C interface defintions for ESP32 and ESP8266
#define I2C_BUS 0
#define I2C_SCL_PIN 14
#define I2C_SDA_PIN 13
#define I2C_FREQ I2C_FREQ_100K

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"
#include <bme680.h>
#include <mqtt_sensor_data.h>

    esp_err_t mqtt_sensor_bme680_get_results_blocking(struct sensor_data *results);

#ifdef __cplusplus
}
#endif

#endif
