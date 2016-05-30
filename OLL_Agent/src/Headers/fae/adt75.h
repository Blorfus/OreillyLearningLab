

/*
	ADT75 Temperature Sensor Interface
*/

#ifndef _ADT75_H
#define _ADT75_H

#include <unistd.h>

#include "i2c.h"

#define REG_TEMP 0x00
#define REG_CONFIG 0x01
#define REG_ONESHOT 0x04

#define D5_ONESHOT 5

typedef struct adt75 {
	uint8_t address;
	float temperature;
	i2c_t* i2cbus;
	uint8_t active;
} adt75_t;

int adt75_init(adt75_t* sensor, i2c_t* i2cbus, uint8_t address);
int adt75_measure(adt75_t* sensor);

#endif
