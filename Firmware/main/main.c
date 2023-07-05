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
#include "esp_wifi_types.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht20.h"
#include "soc/clk_tree_defs.h"
#include "soil_humidity.h"
#include "wifi.h"
#include "leds.h"
#include "database.h"


void app_main(void)
{
    ESP_ERROR_CHECK(dht20_init());
    ESP_ERROR_CHECK(moisture_sensor_init());
    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(leds_init());

    vTaskDelay(10000/portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(database_init());
}
