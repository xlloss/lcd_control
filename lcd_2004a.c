/***************************************************************************
    copyright            : (C) by 2003-2004 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.c,v 1.5 2004/02/29 11:05:28 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.     //    lcd_write_date(lcd_dev, 0x02);                              *
 *                                                                         *
 ***************************************************************************/
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
#include "lcd_2004a.h"
#include <time.h>

static struct timespec sleep_timespec = {.tv_sec = 0, .tv_nsec = 200000000};

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

static inline __s32 i2c_smbus_write_i2c_block_data(int file, __u8 command,
                                               __u8 length, __u8 *values)
{
	union i2c_smbus_data data;
	int i;

	if (length > 32)
		length = 32;

	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];

	data.block[0] = length;

	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
	                        I2C_SMBUS_I2C_BLOCK_DATA, &data);
}

/* Returns the number of read bytes */
static inline __s32 i2c_smbus_block_process_call(int file, __u8 command,
                                                 __u8 length, __u8 *values)
{
	union i2c_smbus_data data;
	int i;

	if (length > 32)
		length = 32;

	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];

	data.block[0] = length;

	if (i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
            I2C_SMBUS_BLOCK_PROC_CALL, &data)) {
		return -1;
    } else {
		for (i = 1; i <= data.block[0]; i++)
			values[i-1] = data.block[i];
		return data.block[0];
	}
}

static int i2c_write_1b(struct lcd *lcd_dev, __u8 buf)
{
	int r;

	/* we must simulate a plain I2C byte write with SMBus functions */
    //printf("%s buf 0x%x\r\n", __func__, buf);
	r = i2c_smbus_write_byte(lcd_dev->fd, buf);
	if(r < 0)
		fprintf(stderr, "Error i2c_write_1b: %s\n", strerror(errno));

	usleep(5);
	return r;
}

static int i2c_write_2b(struct lcd *lcd_dev, __u8 buf[2])
{
	int r;
	/* 
     * we must simulate a plain I2C byte write with SMBus functions
     */
//	r = i2c_smbus_write_byte_data(lcd_dev->fd, buf[0], buf[1]);
//	if(r < 0)
//		fprintf(stderr, "Error i2c_write_2b: %s\n", strerror(errno));
//
//	usleep(10000);
	return r;
}

static int i2c_write_3b(struct lcd *lcd_dev, __u8 buf[3])
{
	int r;

	/* 
     * we must simulate a plain I2C byte write with SMBus functions
	 * the __u16 data field will be byte swapped by the SMBus protocol
     */
//	r = i2c_smbus_write_word_data(e->fd, buf[0], buf[2] << 8 | buf[1]);
//	if(r < 0)
//		fprintf(stderr, "Error i2c_write_3b: %s\n", strerror(errno));
//
//	usleep(10);
	return r;
}


#define CHECK_I2C_FUNC(var, label) \
	do { if (0 == (var & label)) { \
		fprintf(stderr, "\nError: " \
			#label " function is required. Program halted.\n\n"); \
		exit(1); } \
	} while(0);

int lcd_open(char *dev_name, int addr, int type, struct lcd* lcd_dev)
{
	int funcs, fd, r;

	fd = open(dev_name, O_RDWR);
	if (fd <= 0) {
		fprintf(stderr, "Error %s: %s\n", strerror(errno), __func__);
		return -1;
	}

	if ((r = ioctl(fd, I2C_FUNCS, &funcs) < 0)) {
		fprintf(stderr, "Error %s: %s\n", strerror(errno), __func__);
		return -1;
	}

	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_WORD_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA );

	if ((r = ioctl (fd, I2C_SLAVE, addr)) < 0) {
		printf ("%s %d\n", __func__, __LINE__);
		fprintf(stderr, "Error eeprom_open: %s\n", strerror(errno));
		return -1;
	}

	lcd_dev->fd = fd;
	lcd_dev->addr = addr;
	lcd_dev->dev = dev_name;
	lcd_dev->type = type;

	return 0;
}

int lcd_close(struct lcd *lcd_dev)
{
//	close(e->fd);
//	e->fd = -1;
//	e->dev = 0;
//	e->type = EEPROM_TYPE_UNKNOWN;
	return 0;
}

int lcd_read_byte(struct lcd *lcd_dev)
{
//	int r;
//
//	ioctl(e->fd, BLKFLSBUF); // clear kernel read buffer
//
//	r = i2c_smbus_read_byte(lcd_dev->fd);
//
//	return r;
    return 0;
}

#define BL_BIT 3
#define BL_BIT_EN 1
#define BL_BIT_DIS 0

void lcd_backlight(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == BL_BIT_EN)
        lcd_dev->ctl = lcd_dev->ctl | (1 << BL_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << BL_BIT);
}

//Instruction_register=0 / Data_register=1
void lcd_rs(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == RS_BIT_INST)
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << RS_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl | (1 << RS_BIT);
}

#define RW_BIT 1
#define RW_BIT_W 0
#define RW_BIT_R 1
//Read=1/write=0
void lcd_rw(struct lcd *lcd_dev, unsigned char sel)
{

    if (sel == RW_BIT_W)
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << RW_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl | (1 << RW_BIT);
}

#define EN_BIT 2
#define EN_BIT_EN 1
#define EN_BIT_DIS 0

//Enable=1
void lcd_enable(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == EN_BIT_EN)
        lcd_dev->ctl = lcd_dev->ctl | (1 << EN_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << EN_BIT);
}

#define LCD_BUS_4BIT_7_4 4
#define DATA_HI 0xF0
#define DATA_LO 0x0F
#define LCD_BUS_CMD_MASK 0x0F

void write_4bit(struct lcd *lcd_dev, __u8 data)
{
    unsigned char wb_buf = 0;

    wb_buf = wb_buf | ((data & DATA_LO) << LCD_BUS_4BIT_7_4);
    lcd_enable(lcd_dev, EN_BIT_EN);
    wb_buf = wb_buf | lcd_dev->ctl;
    i2c_write_1b(lcd_dev, wb_buf);
    _nanosleep();
    wb_buf = wb_buf & ~(LCD_BUS_CMD_MASK);
    lcd_enable(lcd_dev, EN_BIT_DIS);
    wb_buf = wb_buf | lcd_dev->ctl;
    i2c_write_1b(lcd_dev, wb_buf);
}

void write_data(struct lcd *lcd_dev, __u8 data)
{
    unsigned char wb_buf = 0;

    write_4bit(lcd_dev, (data & DATA_HI) >> LCD_BUS_4BIT_7_4);
    write_4bit(lcd_dev, data & DATA_LO);
    lcd_dev->dat = data;
}

void write_ctl(struct lcd *lcd_dev)
{
    unsigned char wb_buf = 0;

    wb_buf = wb_buf | lcd_dev->dat | lcd_dev->ctl;
    i2c_write_1b(lcd_dev, wb_buf);
}
