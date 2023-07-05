#include "leds.h"
#include "portmacro.h"


#define PI 3.14
#define DELTA_T (1000 / (float) configTICK_RATE_HZ)
#define LEDC_MODE LEDC_LOW_SPEED_MODE

#define SENSOR_LED_PIN_0 27
#define SENSOR_LED_PIN_1 14
#define SENSOR_LED_PIN_2 12
#define OUTPUT_PINS_MASK ((1ULL << SENSOR_LED_PIN_0) \
                         |(1ULL << SENSOR_LED_PIN_1) \
                         |(1ULL << SENSOR_LED_PIN_2))

static SemaphoreHandle_t period_mutex;
static float period = 5000;

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
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = SENSOR_LED_PIN_0,
        .duty = 0,
        .hpoint = 0
    };

    error = ledc_channel_config(&ledc_channel);
    if (error != ESP_OK) {
        return error;
    }

    ledc_channel.channel = LEDC_CHANNEL_1;
    ledc_channel.gpio_num = SENSOR_LED_PIN_1;
    error = ledc_channel_config(&ledc_channel);
    if (error != ESP_OK) {
        return error;
    }

    ledc_channel.channel = LEDC_CHANNEL_2;
    ledc_channel.gpio_num = SENSOR_LED_PIN_2;
    error = ledc_channel_config(&ledc_channel);
    if (error != ESP_OK) {
        return error;
    }

    period_mutex = xSemaphoreCreateMutex();

    return ESP_OK;
}

void update_period(float new_period) {
    printf("Updating period\n");
    xSemaphoreTake(period_mutex, portMAX_DELAY);
    period = new_period;
    xSemaphoreGive(period_mutex);
}

void leds_breathe_task() {
    float phase = 0;

    while (1) {
        xSemaphoreTake(period_mutex, portMAX_DELAY);
        float local_period = period;
        xSemaphoreGive(period_mutex);

        phase += 2 * PI * 1 * DELTA_T / local_period;
        while (phase > 2 * PI) {
            phase -= 2 * PI;
        }
        int new_duty_0 = (int) (1024 * powf((sin(phase)), 2));
        int new_duty_1 = (int) (1024 * powf((sin(phase + 2*PI/3)), 2));
        int new_duty_2 = (int) (1024 * powf((sin(phase + 4*PI/3)), 2));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, new_duty_0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, new_duty_1));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1));

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, new_duty_2));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2));

        vTaskDelay(1);
    }
}
