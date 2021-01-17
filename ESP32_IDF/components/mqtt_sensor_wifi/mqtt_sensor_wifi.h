/* Iperf Example - iperf declaration

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __MQTT_SENSOR_WIFI_H_
#define __MQTT_SENSOR_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_types.h"
#include "esp_err.h"

esp_err_t mqtt_sensor_wifi_connect_to_sta(void);

#ifdef __cplusplus
}
#endif

#endif