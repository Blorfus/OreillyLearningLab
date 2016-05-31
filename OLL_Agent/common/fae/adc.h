

/*
	AD5593R Analog I/O expander interface
*/

#ifndef _ADC_H
#define _ADC_H

#include "i2c.h"

#define ADC_REG_POWER 0x0B
#define ADC_REG_ADCMASK 0x04
#define ADC_REG_ADCSEQ 0x02
#define ADC_REG_ADCDATA 0x40

typedef struct adc {
	char address;
	i2c_t* i2cbus;
	float volts[8];
	uint8_t active;
} adc_t;

int adc_init(adc_t* sensor, i2c_t* i2cbus, uint8_t address);
int adc_measure(adc_t* sensor);
static int adc_write_reg(i2c_t* bus, uint8_t address, uint8_t high, uint8_t low);

#endif
