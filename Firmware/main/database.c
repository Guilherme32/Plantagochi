#include "database.h"
#include "wifi.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 1024
static const char *TAG = "HTTP_CLIENT";

// static const char* host = "192.168.0.112:8000";
static SemaphoreHandle_t host_mutex;
static char host[32] = {0};


bool send_data(int temperature, int soil_humidity, int ambient_humidity)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

    esp_http_client_config_t config = {
        .host = host,
        .path = "/get",
        .query = "esp",
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST
    char address[64];
    xSemaphoreTake(host_mutex, portMAX_DELAY);
    sprintf(address, "http://%s/data/", host);
    xSemaphoreGive(host_mutex);

    char post_data[512] = {0};
    sprintf(post_data,
        "{\"temperature\": %d, \"soil_humidity\": %d, \"ambient_humidity\": %d}",
        temperature, soil_humidity, ambient_humidity
        );

    esp_http_client_set_url(client, address);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Data sent successfully!");
        return true;
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        return false;
    }
}

bool find_host() {
    char base_host[20] = {0};
    if (!get_ip(base_host)) {
        return false;
    }

    for (int i=strlen(base_host)-1; i>0; i--) {        // 192.168.0.1 -> 192.168.0.
        if (base_host[i] == '.') {
            base_host[i+1] = 0;
            break;
        }
    }

    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

    esp_http_client_config_t config = {
        .host = host,
        .path = "/PlantagochiUFJF_GGJJL/",
        .query = "esp",
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
        .timeout_ms = 2000
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char address[64] = {0};
    for (int i=100; i<255; i++) {
        sprintf(address, "http://%s%d:8000/PlantagochiUFJF_GGJJL/", base_host, i);
        esp_http_client_set_url(client, address);

          // GET
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {

            xSemaphoreTake(host_mutex, portMAX_DELAY);
            sprintf(host, "%s%d:8000", base_host, i);
            xSemaphoreGive(host_mutex);

            printf("Host encontrado: %s\n", host);
            return true;
        } else {
            printf("Addr %s address failed \n", address);
        }
    }

    return false;
}

esp_err_t database_init() {
    printf("Trying to send the request...\n");
    host_mutex = xSemaphoreCreateMutex();

    return ESP_OK;
}
