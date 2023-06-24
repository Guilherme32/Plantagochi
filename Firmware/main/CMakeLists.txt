cmake_minimum_required(VERSION 3.26)

idf_component_register(SRCS "main.c" "dht20.c" "soil_humidity.c"
                    INCLUDE_DIRS "")
