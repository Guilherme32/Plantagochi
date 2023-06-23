/* DHT20.H
* The dht20 is a low-cost ambient humidity and temperature sensor with a
* digital interface. This small library was written to use the sensor for a
* university project (Final project, CEL082 UFJF 2023.1)
*
* @author Guilherme Sampaio
*/

#include "driver/i2c.h"
#include <math.h>

/**
* Initializes the sensor. The initialization is for the i2c interface
*/
esp_err_t dht20_init();

/**
* Reads the humidity and temperature using the dht20 sensor
* @param int16_t* humidity Pointer to store the read humidity (in 10 * %RH)
* @param int16_t* temperature Pointer to store the read temperature (in 10 * ºC)
* @return bool true if succesfull, false if failed
*/
bool dht20_read(int16_t* humidity, int16_t* temperature);

