

/*
	TSL2561 Ambient Light Sensor Interface
*/

#ifndef _TSL2561_H
#define _TSL2561_H

#include "i2c.h"

#define TSL_REG_CONTROL 0x00
#define TSL_REG_0L 0x8C
#define TSL_REG_0H 0x8D
#define TSL_REG_1L 0x8E
#define TSL_REG_1H 0x8F

#define TSL_ON 0x03
#define TSL_OFF 0x03

typedef struct tsl {
	uint8_t address;
	unsigned int lux;
	i2c_t* i2cbus;
	uint8_t active;
} tsl_t;

int tsl_init(tsl_t* sensor, i2c_t* i2cbus, uint8_t address);
int tsl_measure(tsl_t* sensor);
static uint8_t read_reg(i2c_t* bus, uint8_t address);
static int tsl_write_reg(i2c_t* bus, uint8_t address, uint8_t val);
static int get_lux(unsigned int ch0, unsigned int ch1);


#endif
