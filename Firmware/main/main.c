/**
* Plantagochi - A basic soil and evironment health data logger for taking
* better care of plants by better understanding their conditions.
* A university course project - (Final project, CEL082 UFJF 2023.1)
*/

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht20.h"
#include "soil_humidity.h"


void test_dth20_task() {
    int16_t humidity;
    int16_t temperature;

    while (true) {
        humidity = 0;
        temperature = 0;

        dht20_read(&humidity, &temperature);
        printf(
            "Humidity: %d.%d%%  (+-3)  |    Temperature: %d.%dºC  (+-0.5)\n",
            humidity / 10,
            humidity % 10,
            temperature / 10,
            temperature % 10
        );
        vTaskDelay(10);
    }
}

void test_moisture_sensor_task() {
    while (true) {
        int moisture = moisture_sensor_read();
        printf(
            "Soil relative humidity: %d.%d\n",
            moisture / 10,
            moisture % 10
            );
        vTaskDelay(10);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(dht20_init());
    ESP_ERROR_CHECK(moisture_sensor_init());

    xTaskCreate(
        test_moisture_sensor_task,
        "Moisture test",
        4096,
        NULL,
        10,
        NULL
        );
}
