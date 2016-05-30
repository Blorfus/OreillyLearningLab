

/*
	SHT21 Temperature and Relative Humidity Sensor.
*/

#ifndef _SHT21_H
#define _SHT21_H

#include "i2c.h"

typedef struct sht21 {
	uint8_t address;
	float temperature_c;
	float temperature_f;
	float humidity;
	i2c_t* i2cbus;
	uint8_t active;
} sht21_t;

int sht21_init(sht21_t* sensor, i2c_t* i2cbus, uint8_t address);
int sht21_measure(sht21_t* sensor);
static int sht21_humidity(sht21_t* sensor, float* humidity);
static int sht21_temp(sht21_t* sensor, float* temp);

#endif
