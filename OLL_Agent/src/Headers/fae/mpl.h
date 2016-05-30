

/*
	MPL3115A2 Barometric Pressure Sensor Interface
*/

#ifndef _MPL_H
#define _MPL_H

#include <unistd.h>

#include "i2c.h"

typedef struct mpl {
	uint8_t address;
	uint8_t active;
	float pressure;
	float altitude;
	i2c_t* i2cbus;
} mpl_t;

int mpl_init(mpl_t* sensor, i2c_t* i2cbus, uint8_t address);
int mpl_measure(mpl_t* sensor);
static float mpl_convert_altitude(float pressure);
static uint8_t mpl_read_reg(i2c_t* bus, uint8_t add);
static void mpl_write_reg(i2c_t* bus, uint8_t add, uint8_t value);

#endif
