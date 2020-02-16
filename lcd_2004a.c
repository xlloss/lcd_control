/***************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
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
#include <time.h>
#include "lcd_2004a.h"
#include "i2c_if.h"


/* -------------------------
  P7 P6 P5 P4 P3 P2 P1 P0
 ---------------------------
  D7 D6 D5 D4 BT E  RW RS
  --------------------------*/

static struct timespec sleep_timespec = {.tv_sec = 0, .tv_nsec = 5000000};


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
	close(lcd_dev->fd);
	lcd_dev->fd = -1;
	lcd_dev->dev = 0;
	return 0;
}



void lcd_backlight(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == LCD_BL_BIT_EN)
        lcd_dev->ctl = lcd_dev->ctl | (1 << LCD_BL_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << LCD_BL_BIT);
}

void lcd_rs(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == RS_BIT_INST)
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << RS_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl | (1 << RS_BIT);
}


void lcd_rw(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == LCD_RW_BIT_W)
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << LCD_RW_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl | (1 << LCD_RW_BIT);
}

void lcd_enable(struct lcd *lcd_dev, unsigned char sel)
{
    if (sel == LCD_EN_BIT_EN)
        lcd_dev->ctl = lcd_dev->ctl | (1 << LCD_EN_BIT);
    else
        lcd_dev->ctl = lcd_dev->ctl & ~(1 << LCD_EN_BIT);
}

void write_4bit(struct lcd *lcd_dev, unsigned char data)
{
    unsigned char wb_buf = 0;

    wb_buf = wb_buf | ((data & DATA_LO) << LCD_BUS_DATA);
    lcd_enable(lcd_dev, LCD_EN_BIT_EN);
    wb_buf = wb_buf | lcd_dev->ctl;
    i2c_write_1b(lcd_dev, wb_buf);
    _nanosleep();
    wb_buf = wb_buf & ~(LCD_BUS_CMD_MASK);
    lcd_enable(lcd_dev, LCD_EN_BIT_DIS);
    wb_buf = wb_buf | lcd_dev->ctl;
    i2c_write_1b(lcd_dev, wb_buf);
}

void write_data(struct lcd *lcd_dev, unsigned char data)
{
    write_4bit(lcd_dev, (data & DATA_HI) >> LCD_BUS_DATA);
    write_4bit(lcd_dev, data & DATA_LO);
    lcd_dev->dat = data;
}

unsigned char read_data(struct lcd *lcd_dev)
{
    int r;

    // clear kernel read buffer
    ioctl(lcd_dev->fd, BLKFLSBUF);
    r = i2c_smbus_read_byte(lcd_dev->fd);
    return r;
}


unsigned char lcd_read_date(struct lcd *lcd_dev, unsigned char data)
{
    return read_data(lcd_dev);
}

void lcd_write_date(struct lcd *lcd_dev, unsigned char data)
{
    write_data(lcd_dev, data);
}
