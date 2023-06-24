/* DHT20.C
* The dht20 is a low-cost ambient humidity and temperature sensor with a
* digital interface. This small library was written to use the sensor for a
* university project (Final project, CEL082 UFJF 2023.1)
*
* @author Guilherme Sampaio
*/

#include "dht20.h"


// Region // Definitions -------------------------------------------------------

// I2C port definitions
#define I2C_PORT 0
#define I2C_MAX_DELAY 100

#define I2C_MASTER_SDA_IO 26
#define I2C_MASTER_SCL_IO 25
#define I2C_MASTER_FREQ_HZ 50000

#define ACK I2C_MASTER_LAST_NACK

// DHT20 Interface definitions (check datasheet for more info on those)
#define DHT20_BASE_ADDR (0x38)
#define DHT20_READ_ADDR (DHT20_BASE_ADDR * 2 + 1)
#define DHT20_WRITE_ADDR (DHT20_BASE_ADDR * 2 + 0)

#define TRIGGER_COMMAND (0xac)
#define TRIGGER_PARAM1 (0x33)
#define TRIGGER_PARAM2 (0x00)


// Region // Static function declarations --------------------------------------

/**
* Initializes the esp i2c peripheral. Parameters used are defined on dht20.c
*/
static esp_err_t i2c_init();

/**
* Resets a dht20 register. A necessary step depending on the device
* status response
*/
static void dht20_reset_register(uint8_t address);

/**
* Gets the status from the dht20 sensor. Returns the entire byte. For info
* on what each bit means, check the datasheet
*/
static uint8_t dht20_get_status();

/**
* Asks the dht20 to make a measurement. This is a needed step before trying
* to read (retrieve) the data
*/
static void dht20_ask_measurement();

/**
* Retrieves the humidity and temperature read by the dht20 sensor
* @param int16_t* humidity Pointer to store the read humidity (in 10 * %RH)
* @param int16_t* temperature Pointer to store the read temperature (in 10 * ºC)
* @return bool true if succesfull, false if failed
*/
static bool dht20_retrieve_measurement(int16_t* humidity, int16_t* temperature);


// Region // Static function definitions ---------------------------------------

static esp_err_t i2c_init() {
    int i2c_master_port = 0;
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t error = i2c_param_config(i2c_master_port, &config);
    if (error != ESP_OK) {
        return error;
    }

    return i2c_driver_install(
        i2c_master_port,
        I2C_MODE_MASTER,
        0,
        0,
        ESP_INTR_FLAG_LEVEL1
    );
}


static void dht20_reset_register(uint8_t address) {
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

    // Rewrite some bytes to the registers
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

static uint8_t dht20_get_status() {
    // Check datasheet for the process
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

static void dht20_ask_measurement() {
    // Check datasheet for the process
    uint8_t status = dht20_get_status();

    if (status != 0x18) {
        dht20_reset_register(0x1b);
        dht20_reset_register(0x1c);
        dht20_reset_register(0x1e);
        return dht20_ask_measurement();
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

static bool dht20_retrieve_measurement(int16_t* humidity, int16_t* temperature) {
    // Check status
    for (int i=0; i<100; i++) {
        uint8_t status = dht20_get_status();

        bool busy = status >> 7 == 1;
        if (!busy) {
            break;
        }

        if (i >= 99) {    // Fail if it takes too long
            return false;
        }
        vTaskDelay(1);
    }

    // Status ok, fetch data
    uint8_t buffer[7] = { 0 };

    i2c_cmd_handle_t retrieve_chain = i2c_cmd_link_create();
    i2c_master_start(retrieve_chain);
    i2c_master_write_byte(retrieve_chain, DHT20_READ_ADDR, ACK);

    i2c_master_read(retrieve_chain, buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(retrieve_chain);

    i2c_master_cmd_begin(I2C_PORT, retrieve_chain, I2C_MAX_DELAY);
    i2c_cmd_link_delete(retrieve_chain);

    // Data fetched, calculate values in RH (%) and ºC
    // The values hold one decimal value (humidity 587 would be 58.7% RH)
    int32_t humidity_calc = 0;
    humidity_calc = (humidity_calc + buffer[1]) << 8;
    humidity_calc = (humidity_calc + buffer[2]) << 8;
    humidity_calc = (humidity_calc + buffer[3]) >> 4;

    humidity_calc = humidity_calc * 10 * 100 / (1<<20);

    int32_t temperature_calc = 0;
    temperature_calc = (temperature_calc + (buffer[3] & 0x0f)) << 8;
    temperature_calc = (temperature_calc + buffer[4]) << 8;
    temperature_calc = (temperature_calc + buffer[5]);

    temperature_calc = temperature_calc * 10 * 200 / (1<<20) - 500;

    // Values calculated, write them to the pointers
    *humidity = (int16_t) humidity_calc;
    *temperature = (int16_t) temperature_calc;

    // Operation successfull
    return true;
}


// Region // Public function definitions ---------------------------------------

esp_err_t dht20_init() {
    return i2c_init();
}


bool dht20_read(int16_t* humidity, int16_t* temperature) {
    dht20_ask_measurement();
    vTaskDelay(80/portTICK_PERIOD_MS);
    return dht20_retrieve_measurement(humidity, temperature);
}


