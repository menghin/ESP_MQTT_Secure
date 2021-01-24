idf_component_register(SRCS "mqtt_sensor_bme680.c"
                    INCLUDE_DIRS .
                    REQUIRES ulp bme680 mqtt_sensor_data)
