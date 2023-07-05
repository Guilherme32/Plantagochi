#include "sensor_reader.h"


float sensors_to_breathe_period(int temperature, int ambient_humidity, int soil_humidity) {
    float temp_factor, amb_hum_factor, soil_factor;

    if (temperature > 200 && temperature < 350) {
        temp_factor = 1;
    } else {
        temp_factor = 0.6;
    }

    if (ambient_humidity > 300 && ambient_humidity < 900) {
        amb_hum_factor = 1;
    } else {
        amb_hum_factor = 0.6;
    }

    if (soil_humidity > 200 && soil_humidity < 900) {
        soil_factor = 1;
    } else {
        soil_factor = 0.3;
    }

    float total_factor =  temp_factor * amb_hum_factor * soil_factor;

    return (2000) + (14000 - 2000) * total_factor;
}

void sensor_reader_task() {
    int retries = 0;

    while (1) {
        int temperature, ambient_humidity, soil_humidity;

        dht20_read(&ambient_humidity, &temperature);        // Read
        soil_humidity = moisture_sensor_read();
        
        float breathe_period = sensors_to_breathe_period(      // Update breath
            temperature,
            ambient_humidity,
            soil_humidity
        );
        update_period(breathe_period);

        printf(                                                    // prints
            "Humidity: %d.%d%%  (+-3)  |    Temperature: %d.%dºC  (+-0.5)\n",
            ambient_humidity / 10,
            ambient_humidity % 10,
            temperature / 10,
            temperature % 10
        );

        printf(
            "Soil relative humidity: %d.%d\n",
            soil_humidity / 10,
            soil_humidity % 10
            );

                                    // Sends the data
        if (send_data(temperature, ambient_humidity, soil_humidity)) {
            retries = 0;
        } else {
            retries ++;
        }

        if (retries >= 10) {
            printf("Could not find host 10 times in a row, trying to find it again\n");
            find_host();
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);            // wait for new cycle
    }
}



