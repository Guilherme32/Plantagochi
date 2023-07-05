cmake_minimum_required(VERSION 3.26)

idf_component_register(SRCS "main.c" "dht20.c" "soil_humidity.c" "wifi.c" "leds.c" "database.c" "sensor_reader.c"
                    INCLUDE_DIRS "")
