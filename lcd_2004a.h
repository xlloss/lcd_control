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

#define LCD_BUS_UNKNOWN 0
#define LCD_BUS_4BIT_ADDR 0x1
#define LCD_BUS_8BIT_ADDR 0x2

#define _nanosleep() nanosleep(&sleep_timespec, NULL)

struct lcd
{
    char *dev;
    int addr;
    int fd;
    int type;
    unsigned char ctl;
    unsigned char dat;
};

#define LCD_CMD_INIT 0x03
#define LCD_BUSTYPE_8BIT 0
#define LCD_BUSTYPE_4BIT 1

#define LCD_BUS_DATA 4
#define DATA_HI 0xF0
#define DATA_LO 0x0F
#define LCD_BUS_CMD_MASK 0x0F


int lcd_open(char *dev_fqn, int addr, int type, struct lcd*);
int lcd_close(struct lcd *lcd_dev);
int lcd_read_byte(struct lcd *lcd_dev);
void write_4bit(struct lcd *lcd_dev, __u8 data);
void write_data(struct lcd *lcd_dev, __u8 data);
void write_ctl(struct lcd *lcd_dev);

#define RS_BIT 0
#define RS_BIT_INST 0
#define RS_BIT_DATA 1

void lcd_rs(struct lcd *lcd_dev, unsigned char sel);


#define LCD_BL_BIT 3
#define LCD_BL_BIT_EN 1
#define LCD_BL_BIT_DIS 0

void lcd_backlight(struct lcd *lcd_dev, unsigned char sel);


#define LCD_EN_BIT 2
#define LCD_EN_BIT_EN 1
#define LCD_EN_BIT_DIS 0

void lcd_enable(struct lcd *lcd_dev, unsigned char sel);

#define LCD_RW_BIT 1
#define LCD_RW_BIT_W 0
#define LCD_RW_BIT_R 1
void lcd_rw(struct lcd *lcd_dev, unsigned char sel);

