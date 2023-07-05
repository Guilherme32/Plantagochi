/* SOIL_HUMIDITY.H
* This small library implements the interface to read the generic capacitor
* soil humidity sensor. The sensor doesn't have a model, or a name (not that I
* could find, at least). The most identification I can give on it is that it
* is a black board with "Capacitive Soil Moisture sensor V2.0" written on it.
* This small library was written to use the sensor for a university project
* (Final project, CEL082 UFJF 2023.1)
*
* @author Guilherme Sampaio
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "sdkconfig.h"


/**
* Initializes the sensor. The initialization is for the esp adc, in oneshot
* mode
*/
esp_err_t moisture_sensor_init();

/**
* Reads the sensor. It is read with a multisampling of 64 points for better
* stability
* @return int The soil relative humidity, in RH % * 10
*/
int moisture_sensor_read();

/**
* Task for testing the moisture sensor. Continually collect and prints the
* sensor reading
*/
void test_moisture_sensor_task();
