/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "hal/i2c_types.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "driver/i2c.h"


#define I2C_MASTER_SDA_IO 1
#define I2C_MASTER_SCL_IO 2
#define I2C_MASTER_FREQ_HZ 50000

esp_err_t i2c_init() {
    int i2c_master_port = 0;
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,         // select SDA GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,         // select SCL GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,  // select frequency specific to your project
        .clk_flags = 0,                          // optional; you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
    };

    i2c_param_config(i2c_master_port, &config);

    return i2c_driver_install(i2c_master_port, I2C_MODE_MASTER, NULL, NULL, ESP_INTR_FLAG_LEVEL1);
}

#define DHT20_ADDR 0x38
#define DHT20_READ_ADDR (DHT20_ADDR * 2 + 1)
#define DHT20_WRITE_ADDR (DHT20_ADDR * 2 + 0)

void i2c_chain() {
    i2c_cmd_handle_t chain = i2c_cmd_link_create();
    i2c_master_start(chain);
    
}

void i2c_test() {
    ESP_ERROR_CHECK(i2c_init());
}

void app_main(void)
{
    printf("Heello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
