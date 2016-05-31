
/*
	Linux I2C interface library
*/

#ifndef _I2C_H
#define _I2C_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#define I2C_OPEN_SUCCESS 1
#define I2C_OPEN_FAILURE 0

#define I2C_OPEN 1
#define I2C_CLOSED 0

typedef struct i2c {
	char device[40];
	uint8_t open;
	int file;
} i2c_t;

/* Set up and open an I2C bus */
int i2c_init(i2c_t* bus, uint8_t *device);

/* Close the i2c port */
int i2c_close(i2c_t* bus);

#endif
