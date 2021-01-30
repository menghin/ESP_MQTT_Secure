idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt_sensor_sntp mqtt_sensor_data mqtt_sensor_wifi mqtt_sensor_bme680 mqtt_sensor_mqtt)
