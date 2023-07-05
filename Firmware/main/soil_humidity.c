/* SOIL_HUMIDITY.C
* This small library implements the interface to read the generic capacitor
* soil humidity sensor. The sensor doesn't have a model, or a name (not that I
* could find, at least). The most identification I can give on it is that it
* is a black board with "Capacitive Soil Moisture sensor V2.0" written on it.
* This small library was written to use the sensor for a university project
* (Final project, CEL082 UFJF 2023.1)
*
* @author Guilherme Sampaio
*/

#include "soil_humidity.h"


// Region // Definitions -------------------------------------------------------

#define MOISTURE_ADC_CHANNEL ADC_CHANNEL_5

// Region // Global variables

static adc_oneshot_unit_handle_t adc_handle;
static bool initiated = false;

// Region // Static function declarations --------------------------------------

/**
* Transforms the analog reading to a relative humidity value, in RH % * 10
* The internet says the sensor is not precise at all. I couldn't find a
* datasheet to fact check that. To be safe, use the returned value as a
* general indication, and not as a precise measurement
* @param adc_read int The adc reading, with 11dB of attenuation
* @return int The relative humidity, in RH % * 10
*/
static int reading_to_relative_humidity(int adc_read);


// Region // Static function definitions ---------------------------------------

static int reading_to_relative_humidity(int adc_read) {
    // Calibration values
    int dry = 3500;        // On open air
    int wet = 1470;        // Submerged in water

    if (adc_read > dry) {
        return 0;
    }
    if (adc_read < wet) {
        return 1000;
    }

    return (adc_read - dry) * 100 * 10 / (wet - dry);
}


// Region // Public function definitions ---------------------------------------

esp_err_t moisture_sensor_init() {
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t error = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (error != ESP_OK) {
        return error;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11
    };

    error = adc_oneshot_config_channel(
        adc1_handle,
        MOISTURE_ADC_CHANNEL,
        &config
        );
    if (error != ESP_OK) {
        return error;
    }

    adc_handle = adc1_handle;
    initiated = true;

    return ESP_OK;
}

int moisture_sensor_read() {
    uint32_t reading_mean = 0;

    for (int i=0; i<64; i++) {
        int reading = 100;
        adc_oneshot_read(adc_handle, MOISTURE_ADC_CHANNEL, &reading);
        reading_mean += reading;
    }
    reading_mean /= 64;

    int relative = reading_to_relative_humidity(reading_mean);

    return relative;
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
