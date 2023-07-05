#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <math.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

esp_err_t leds_init();
void update_period(float new_period);
void leds_breathe_task();
