/* Iperf Example - iperf declaration

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef __MQTT_SENSOR_DATA_H_
#define __MQTT_SENSOR_DATA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define MQTT_SENSOR_DATA_BUFFER_SIZE 375

    struct sensor_data
    {
        time_t timestamp;
        float temperature;
        float humidity;
        float pressure;
        float gasResistance;
    };

    uint32_t mqtt_sensor_data_count(void);
    void mqtt_sensor_data_push(struct sensor_data *item);
    void mqtt_sensor_data_get_last(struct sensor_data *item);
    void mqtt_sensor_data_drop();

#ifdef __cplusplus
}
#endif

#endif
