/* Iperf Example - iperf declaration

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __MQTT_SENSOR_MQTT_H_
#define __MQTT_SENSOR_MQTT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"
#include "mqtt_client.h"
#include <mqtt_sensor_data.h>

    esp_err_t mqtt_sensor_mqtt_connect(esp_mqtt_client_config_t mqtt_cfg, char *mqtt_pub_topic);
    esp_err_t mqtt_sensor_mqtt_disconnect(void);
    esp_err_t mqtt_sensor_mqtt_publish(struct sensor_data *results);

#ifdef __cplusplus
}
#endif

#endif
