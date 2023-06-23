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

#include "driver/i2c.h"
#include <math.h>


#define I2C_PORT 0
#define I2C_MAX_DELAY 100

#define I2C_MASTER_SDA_IO 26
#define I2C_MASTER_SCL_IO 25
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

    return i2c_driver_install(i2c_master_port, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1);
}

#define DHT20_BASE_ADDR (0x38)
#define DHT20_READ_ADDR (DHT20_BASE_ADDR * 2 + 1)
#define DHT20_WRITE_ADDR (DHT20_BASE_ADDR * 2 + 0)

#define ACK I2C_MASTER_LAST_NACK

#define TRIGGER_COMMAND (0xac)
#define TRIGGER_PARAM1 (0x33)
#define TRIGGER_PARAM2 (0x00)


void reset_dht20_register(uint8_t address) {
    uint8_t buffer[4] = { 0 };

    // Reset to 0
    i2c_cmd_handle_t reset_chain = i2c_cmd_link_create();

    i2c_master_start(reset_chain);
    i2c_master_write_byte(reset_chain, DHT20_WRITE_ADDR, ACK);
    i2c_master_write_byte(reset_chain, address, ACK);
    i2c_master_write_byte(reset_chain, 0x00, ACK);
    i2c_master_write_byte(reset_chain, 0x00, ACK);
    i2c_master_stop(reset_chain);

    i2c_master_cmd_begin(I2C_PORT, reset_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(reset_chain);

    vTaskDelay(1 + 5/portTICK_PERIOD_MS);

    // Read some bytes
    i2c_cmd_handle_t read_chain = i2c_cmd_link_create();

    i2c_master_start(read_chain);
    i2c_master_write_byte(read_chain, DHT20_READ_ADDR, ACK);
    i2c_master_read(read_chain, buffer, 3, ACK);
    i2c_master_stop(read_chain);

    i2c_master_cmd_begin(I2C_PORT, read_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(read_chain);

    vTaskDelay(1 + 10/portTICK_PERIOD_MS);

    i2c_cmd_handle_t rewrite_chain = i2c_cmd_link_create();

    i2c_master_start(rewrite_chain);
    i2c_master_write_byte(rewrite_chain, DHT20_WRITE_ADDR, ACK);
    i2c_master_write_byte(rewrite_chain, 0xb0 | address, ACK);
    i2c_master_write_byte(rewrite_chain, buffer[1], ACK);
    i2c_master_write_byte(rewrite_chain, buffer[2], ACK);
    i2c_master_stop(rewrite_chain);

    i2c_master_cmd_begin(I2C_PORT, rewrite_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(rewrite_chain);
}

uint8_t dht20_get_status() {
    uint8_t status = 0;

    i2c_cmd_handle_t chain = i2c_cmd_link_create();
    i2c_master_start(chain);
    i2c_master_write_byte(chain, DHT20_READ_ADDR, ACK);

    i2c_master_read(chain, &status, 1, ACK);
    i2c_master_stop(chain);

    i2c_master_cmd_begin(I2C_PORT, chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(chain);

    return status;
}

void i2c_ask_measurement() {
    // Check datasheet for the process
    uint8_t status = dht20_get_status();

    if (status != 0x18) {
        reset_dht20_register(0x1b);
        reset_dht20_register(0x1c);
        reset_dht20_register(0x1e);
        return i2c_ask_measurement();
    }

    vTaskDelay(1 + 10/portTICK_PERIOD_MS);

    i2c_cmd_handle_t trigger_chain = i2c_cmd_link_create();
    i2c_master_start(trigger_chain);
    i2c_master_write_byte(trigger_chain, DHT20_WRITE_ADDR, ACK);

    i2c_master_write_byte(trigger_chain, TRIGGER_COMMAND, ACK);
    i2c_master_write_byte(trigger_chain, TRIGGER_PARAM1, ACK);
    i2c_master_write_byte(trigger_chain, TRIGGER_PARAM2, ACK);
    i2c_master_stop(trigger_chain);

    i2c_master_cmd_begin(I2C_PORT, trigger_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(trigger_chain);
}

void i2c_retrieve_measurement() {
    while (true) {
        uint8_t status = dht20_get_status();

        bool busy = status >> 7 == 1;
        if (!busy) {
            break;
        }
        vTaskDelay(1);
    }

    uint8_t buffer[7] = { 0 };

    i2c_cmd_handle_t retrieve_chain = i2c_cmd_link_create();
    i2c_master_start(retrieve_chain);
    i2c_master_write_byte(retrieve_chain, DHT20_READ_ADDR, ACK);

    i2c_master_read(retrieve_chain, buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(retrieve_chain);

    i2c_master_cmd_begin(I2C_PORT, retrieve_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(retrieve_chain);

    uint32_t humidity = 0;
    humidity = (humidity + buffer[1]) << 8;
    humidity = (humidity + buffer[2]) << 8;
    humidity = (humidity + buffer[3]) >> 4;

    humidity = humidity * 100 / (1<<20);// powf(2, ((float)humidity/20)) * 100;

    uint32_t temperature = 0;
    temperature = (temperature + (buffer[3] & 0x0f)) << 8;
    temperature = (temperature + buffer[4]) << 8;
    temperature = (temperature + buffer[5]);

    temperature = temperature * 200 / (1<<20) - 50;   // powf(2, ((float)temperature/20)) * 200 - 50;

    printf(
        "H: %lu%%    |    T: %luºC\n",
        (unsigned long int)humidity,
        (unsigned long int)temperature
        );
}



void read_dht20() {
    i2c_ask_measurement();
    vTaskDelay(3);
    i2c_retrieve_measurement();
}

void i2c_test() {
    ESP_ERROR_CHECK(i2c_init());
    read_dht20();
}

void app_main(void)
{
    printf("Heello world!\n");
    i2c_test();

    while (true) {
        read_dht20();
        vTaskDelay(10);
    }
}
