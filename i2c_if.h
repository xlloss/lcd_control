#ifndef _I2C_IF_H
#define _I2C_IF_H 

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "lcd_2004a.h"

int i2c_write_1b(struct lcd *lcd_dev, __u8 buf);

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command, 
                                     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	return ioctl(file, I2C_SMBUS, &args);
}


static inline __s32 i2c_smbus_read_byte(int file)
{
	union i2c_smbus_data data;

	if (i2c_smbus_access(file, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data))
		return -1;
	else
		return 0xFF & data.byte;
}

static inline __s32 i2c_smbus_write_byte(int file, __u8 value)
{
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, value,
	                        I2C_SMBUS_BYTE, NULL);
}

static inline __s32 i2c_smbus_write_byte_data(int file, __u8 command, 
                                              __u8 value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
	                        I2C_SMBUS_BYTE_DATA, &data);
}

static inline __s32 i2c_smbus_read_word_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(file, I2C_SMBUS_READ, command,
	                     I2C_SMBUS_WORD_DATA, &data))
		return -1;
	else
		return 0xFFFF & data.word;
}

static inline __s32 i2c_smbus_write_word_data(int file, __u8 command, 
                                              __u16 value)
{
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
	                        I2C_SMBUS_WORD_DATA, &data);
}

#endif
