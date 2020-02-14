/***************************************************************************
    copyright            : (C) by 2003-2004 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.h,v 1.6 2004/02/29 11:05:28 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define LCD_BUS_UNKNOWN	0
#define LCD_BUS_4BIT_ADDR	0x1
#define LCD_BUS_8BIT_ADDR 	0x2

#define _nanosleep() nanosleep(&sleep_timespec, NULL)

struct lcd
{
    char *dev; // device file i.e. /dev/i2c-N
    int addr; // i2c address
    int fd; // file descriptor
    int type; // eeprom type
    unsigned char ctl;
    unsigned char dat;
};

/*
 * opens the eeprom device at [dev_fqn] (i.e. /dev/i2c-N) whose address is
 * [addr] and set the eeprom_24c32 [e]
 */
int lcd_open(char *dev_fqn, int addr, int type, struct lcd*);
/*
 * closees the eeprom device [e] 
 */
int lcd_close(struct lcd *lcd_dev);
/*
 * read and returns the eeprom byte at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int lcd_read_byte(struct lcd *lcd_dev);
/*

/*
 * writes [data] at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */

void write_4bit(struct lcd *lcd_dev, __u8 data);
int lcd_write_byte(struct lcd *lcd_dev, __u8 data);


void write_data(struct lcd *lcd_dev, __u8 data);
void write_ctl(struct lcd *lcd_dev);

#define RS_BIT 0
#define RS_BIT_INST 0
#define RS_BIT_DATA 1

void lcd_rs(struct lcd *lcd_dev, unsigned char sel);


#define BL_BIT 3
#define BL_BIT_EN 1
#define BL_BIT_DIS 0

void lcd_backlight(struct lcd *lcd_dev, unsigned char sel);
