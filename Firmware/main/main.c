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

#include <math.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

#define PI 3.14
#define DELTA_T (1000 / (double) configTICK_RATE_HZ)
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0

#define WIFI_LED_PIN 27
#define OUTPUT_PINS_MASK ((1ULL << WIFI_LED_PIN))


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
    ESP_ERROR_CHECK(wifi_init());

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = WIFI_LED_PIN,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    double phase = 0;
    while (1) {
        for(int i=1; i<10; i++) {
            double period = i * 1000;

            for (int j=0; j<31490/portTICK_PERIOD_MS; j++) {
                phase += 2 * PI * DELTA_T / period;
                // printf("Time: %f \n", time);
                while (phase > 2 * PI) {
                    phase -= 2 * PI;
                }
                int new_duty = (int) (1024 * powf((sin(phase)), 2));

                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, new_duty));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

                vTaskDelay(1);
            }
        }
    }
}
