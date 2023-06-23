/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
jjjj *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "hal/i2c_types.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "dht20.h"

void app_main(void)
{
    printf("Heello world!\n");
    dht20_init();
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
