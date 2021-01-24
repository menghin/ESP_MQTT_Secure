idf_component_register(SRCS "mqtt_sensor_mqtt.c"
                    INCLUDE_DIRS .
                    REQUIRES ulp mqtt json mqtt_sensor_data)
