#include "wifi.h"
#include "esp_wifi_default.h"

#define SSID "WifiGuilherme"
#define PASSWORD "uhvg8322"
#define MAX_RETRY 10000

#define WIFI_LED_PIN 13
#define LED_MASK (1ULL << WIFI_LED_PIN)

static const char *TAG = "WIFI";

static int retries = 0;
// Should have a mutex / critical section on ip accesses
static char ip[32] = {0}; 


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ip[0] = 0;
        if (retries < MAX_RETRY) {
            esp_wifi_connect();
            retries++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } 
        ESP_LOGI(TAG,"connect to the AP fail");
        gpio_set_level(WIFI_LED_PIN, 0);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        sprintf(ip, ""IPSTR, IP2STR(&event->ip_info.ip));
        gpio_set_level(WIFI_LED_PIN, 1);
        retries = 0;
    }
}

bool get_ip(char* out_ip) {
    if (ip[0] == 0) {
        return false;
    }

    sprintf(out_ip, "%s", ip);
    return true;
}

esp_err_t wifi_init(void)
{
    //Initialize NVS
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      error = nvs_flash_init();
    }
    ESP_ERROR_CHECK(error);

    // Initialize the wifi station
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // Setup led
    gpio_config_t pin_config;
    pin_config.intr_type = GPIO_INTR_DISABLE;
    pin_config.mode = GPIO_MODE_OUTPUT;
    pin_config.pin_bit_mask = LED_MASK;
    pin_config.pull_down_en = 0;
    pin_config.pull_up_en = 0;
    gpio_config(&pin_config);
    gpio_set_level(WIFI_LED_PIN, 0);

    return ESP_OK;
}
