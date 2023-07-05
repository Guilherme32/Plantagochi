#include "leds.h"


#define PI 3.14
#define DELTA_T (1000 / (double) configTICK_RATE_HZ)
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0

#define WIFI_LED_PIN 27
#define OUTPUT_PINS_MASK ((1ULL << WIFI_LED_PIN))


esp_err_t leds_init() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t error = ledc_timer_config(&ledc_timer);
    if (error != ESP_OK) {
        return error;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = WIFI_LED_PIN,
        .duty = 0,
        .hpoint = 0
    };

    error = ledc_channel_config(&ledc_channel);
    if (error != ESP_OK) {
        return error;
    }

    return ESP_OK;
}

void leds_breathe_task() {
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
