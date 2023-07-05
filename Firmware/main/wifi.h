#include <string.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_wifi.h"

bool get_ip(char* out_ip);
esp_err_t wifi_init();

