/***************************************************************************
    copyright            : (C) by 2003-2004 Stefano Barbato
    email                : stefano@codesink.org
    website		 : http://codesink.org/eeprog.html

    $Id: eeprog.c,v 1.28 2004/02/29 11:06:41 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "lcd_2004a.h"
#include <time.h>

#define IIC_DEV "/dev/i2c-2"

#define INIT_MASK 0x01
#define WRITE_INSTR_MASK 0x02
#define WRITE_DATA_MASK 0x04
//#define READ_MASK 0x04
#define LCM2004A_ADDR 0x20

static struct timespec sleep_timespec = {.tv_sec = 0, .tv_nsec = 20000000};
struct lcd lcd_dev;

#define usage_if(a) do { do_usage_if( a , __LINE__); } while(0);
void do_usage_if(int b, int line)
{
	const static char *eeprog_usage = 
		"I2C-24C08(256 bytes) Read/Write Program, ONLY FOR TEST!\n"
		"Base on 'eeprog' by Stefano Barbato (http://codesink.org/eeprog.html)\n"
		"i.e : i2c -r, i2c-w\r\n"
		"FriendlyARM Computer Tech. 2009\n"
		"Modify by Slash. 2016\n"
		"Modify by Slash. 2018\n";
	if(!b)
		return;
	fprintf(stderr, "%s\n[line %d]\n", eeprog_usage, line);
	exit(1);
}


#define die_if(a, msg) do { do_die_if( a , msg, __LINE__); } while(0);
void do_die_if(int b, char* msg, int line)
{
	if(!b)
		return;
	fprintf(stderr, "Error at line %d: %s\n", line, msg);
	fprintf(stderr, "sysmsg: %s\n", strerror(errno));
	exit(1);
}


/* -------------------------
  P7 P6 P5 P4 P3 P2 P1 P0
 ---------------------------
  D7 D6 D5 D4 BT E  RW RS
  --------------------------*/


void lcd_write_date(struct lcd *lcd_dev, unsigned char data)
{
    write_data(lcd_dev, data);
}

int lcd_init(struct lcd *lcd_dev)
{
    lcd_dev->ctl = 0;
    lcd_dev->dat = 0;
    lcd_backlight(lcd_dev, BL_BIT_EN);

    write_4bit(lcd_dev, 0x03);
    _nanosleep();
    
    write_4bit(lcd_dev, 0x03);
    _nanosleep();
    
    write_4bit(lcd_dev, 0x03);
    _nanosleep();

    lcd_write_date(lcd_dev, 0x02);
    lcd_write_date(lcd_dev, 0x2C);
    lcd_write_date(lcd_dev, 0x01);
    lcd_write_date(lcd_dev, 0x0F);
    lcd_write_date(lcd_dev, 0x06);

    return 0;
}

int main(int argc, char** argv)
{
    int option_index = 0;
    int opts;
    unsigned char data;
    unsigned char ctl;
    unsigned char w_cmd = 0;
    unsigned char cmd = 0;

    /* getopt_long stores the option index here. */
    struct option long_options[] = {
        {"start", no_argument, 0, 's'},
        {"instr", required_argument, 0, 'i'},
        {"data", required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    while (1) {
        opts = getopt_long (argc, argv, "si:d:",
        long_options, &option_index);

        /* Detect the end of the options. */
        if (opts == -1)
            break;

        switch (opts) {
        case 's':
            cmd = cmd | INIT_MASK;
            break;
        case 'i':
            cmd = cmd | WRITE_INSTR_MASK;
            w_cmd = strtol(optarg, NULL, 16);
            break;

        case 'd':
            cmd = cmd | WRITE_DATA_MASK;
            w_cmd = strtol(optarg, NULL, 16);
            break;

//        case 'r':
//            cmd = cmd | READ_MASK;
//            break;
    
        case '?':
            break;
        default:
                break;
        }
    }

    if (!cmd) {
        printf("command error\r\n");
        return 0;
    }

    if (lcd_open(IIC_DEV,
            LCM2004A_ADDR, LCD_BUS_4BIT_ADDR, &lcd_dev) < 0) {
        printf ("lcd_open fail\n");
        return 0;
    }

    switch (cmd) {

    case INIT_MASK:
        fprintf(stderr, "LCD init\n");
        lcd_init(&lcd_dev);
        break;

    case WRITE_INSTR_MASK:
        fprintf(stderr, "Writing instruction 0x%x\n", w_cmd);
        //lcd_dev.ctl = lcd_dev.ctl | (1 << BL_BIT);
        lcd_backlight(&lcd_dev, BL_BIT_EN);
        //lcd_dev.ctl = lcd_dev.ctl & ~(1 << RS_BIT);
        lcd_rs(&lcd_dev, RS_BIT_INST);
        lcd_write_date(&lcd_dev, w_cmd);
        break;

    case WRITE_DATA_MASK:
        fprintf(stderr, "Writing data 0x%x\n", w_cmd);
        lcd_backlight(&lcd_dev, BL_BIT_EN);
        lcd_rs(&lcd_dev, RS_BIT_DATA);
        lcd_write_date(&lcd_dev, w_cmd);
        break;

    default:
        printf("?\r\n");
        break;
    }

    lcd_close(&lcd_dev);

	return 0;
}

