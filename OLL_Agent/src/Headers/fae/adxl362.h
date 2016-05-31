

/*
	ADT75 Temperature Sensor Interface
*/

#ifndef _ADXL362_H
#define _ADXL362_H

#define ADXL_READ 0x0B
#define ADXL_WRITE 0x0A
#define ADXL_MEASURE_ON 0x02
#define ADXL_MEASURE_OFF 0xFC

#define ADXL_REG_RESET 0x1F
#define ADXL_REG_POWER 0x2D
#define ADXL_REG_XL 0x0E

#include <stdint.h>
#include <bcm2835.h>
#include <stdio.h>

typedef struct adxl {
	float x,y,z;
	uint8_t active;
} adxl_t;

int adxl_init(adxl_t* sensor);
int adxl_measure(adxl_t* sensor);

static uint8_t adxl_read_reg(uint8_t add);
static void adxl_write_reg(uint8_t add, uint8_t value);

#endif
