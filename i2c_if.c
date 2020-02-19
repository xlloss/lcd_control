/*************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "i2c_if.h"


int i2c_write_1b(struct lcd *lcd_dev, __u8 buf)
{
    int r;

    /* we must simulate a plain I2C byte write with SMBus functions */
    r = i2c_smbus_write_byte(lcd_dev->fd, buf);
    if(r < 0)
        fprintf(stderr, "Error i2c_write_1b: %s\n", strerror(errno));

    usleep(1);
    return r;
}

int i2c_write_2b(struct lcd *lcd_dev, __u8 buf[2])
{
    int r;
    /*
    * we must simulate a plain I2C byte write with SMBus functions
    */
    r = i2c_smbus_write_byte_data(lcd_dev->fd, buf[0], buf[1]);
    if(r < 0)
        fprintf(stderr, "Error i2c_write_2b: %s\n", strerror(errno));

    usleep(10000);
    return r;
}

int i2c_write_3b(struct lcd *lcd_dev, __u8 buf[3])
{
	int r;

	/*
     * we must simulate a plain I2C byte write with SMBus functions
	 * the __u16 data field will be byte swapped by the SMBus protocol
     */
	r = i2c_smbus_write_word_data(lcd_dev->fd, buf[0], buf[2] << 8 | buf[1]);
	if(r < 0)
		fprintf(stderr, "Error i2c_write_3b: %s\n", strerror(errno));

	usleep(10);
	return r;
}
