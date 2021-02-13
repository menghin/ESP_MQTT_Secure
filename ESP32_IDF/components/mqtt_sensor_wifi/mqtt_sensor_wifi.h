/* Iperf Example - iperf declaration

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __MQTT_SENSOR_WIFI_H_
#define __MQTT_SENSOR_WIFI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

    /** @brief Configuration required to connect to the wifi station */
    typedef struct
    {
        uint8_t ssid[32];     /**< SSID of target AP. Null terminated string. */
        uint8_t password[64]; /**< Password of target AP. Null terminated string.*/
        uint8_t channel;      /**< channel of target AP. Set to 1~13 to scan starting from the specified channel before connecting to AP. If the channel of AP is unknown, set it to 0.*/
    } mqtt_sensor_wifi_config_t;

    esp_err_t mqtt_sensor_wifi_disconnect_to_sta(void);
    esp_err_t mqtt_sensor_wifi_connect_to_sta(mqtt_sensor_wifi_config_t wifi_config);

#ifdef __cplusplus
}
#endif

#endif
