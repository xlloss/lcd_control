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
#include <time.h>
#include "lcd_2004a.h"

#define IIC_DEV "/dev/i2c-2"

#define INIT_MASK 0x01
#define WRITE_INSTR_MASK 0x02
#define WRITE_DATA_MASK 0x04
#define READ_DATA_MASK 0x08
#define LCM2004A_ADDR 0x20
#define TEST_MASK 0x10

#define LCD_CMD_CLEAR_DISP 0x01
#define LCD_CMD_RETURN_HOME 0x02
#define LCD_CMD_ENTRY_SET 0x04
#define DISPLAY_SHIFT 0
#define DDRAM_I_D 1

#define LCD_CMD_DISP_CTRL 0x08
#define DISPLAY_ON_OFF 2
#define DISPLAY_ON (1 << DISPLAY_ON_OFF)
#define CURSOR_DISPLAY 1
#define CURSOR_ON (1 << CURSOR_DISPLAY)
#define CURSOR_BLINK 0
#define CURSOR_BLINK_ON (1 << CURSOR_BLINK)

#define LCD_CMD_CUR_DISP_SHIFT 0x10
#define CURSOR_DISPLAY_SHIFT 3
#define SHIFT_RIGHT_LEFT 2

#define LCD_CMD_FUNCTION_SET 0x20
#define BUS_TYPE 4
#define BUS_TYPE_8BIT (1 << BUS_TYPE)

#define DISPLAT_TYPE 3
#define DISPLAT_2_LINE (1 << DISPLAT_TYPE)
#define CHAR_TYPE 2
#define CHAR_5X10_TYPE (1 << CHAR_TYPE)

#define LCD_CMD_SET_CG_ADDR 0x40
#define DISP_LOCATE_1ST ((0x80 & ~0x7F) | 0x0)
#define DISP_LOCATE_2SED ((0x80 & ~0x7F) | 0x40)
#define DISP_LOCATE_3TH ((0x80 & ~0x7F) | 0x14)
#define DISP_LOCATE_4TH ((0x80 & ~0x7F) | 0x54)

//#define LCD_CMD_READ_BF_AC
//#define LCD_CMD_WRITE_DATA_RAM
//#define LCD_CMD_READ_DATA_RAM


static struct timespec sleep_timespec = {.tv_sec = 0, .tv_nsec = 20000000};
struct lcd lcd_dev;

#define usage_if(a) do { do_usage_if( a , __LINE__); } while(0);
void do_usage_if(int b, int line)
{
	const static char *lcd_usage = "lcd usage";

	if(!b)
		return;

	fprintf(stderr, "%s\n[line %d]\n", lcd_usage, line);
	exit(1);
}

int lcd_init(struct lcd *lcd_dev)
{
    lcd_dev->ctl = 0;
    lcd_dev->dat = 0;
    lcd_backlight(lcd_dev, LCD_BL_BIT_EN);

    write_4bit(lcd_dev, LCD_CMD_INIT);
    _nanosleep();
    
    write_4bit(lcd_dev, LCD_CMD_INIT);
    _nanosleep();
    
    write_4bit(lcd_dev, LCD_CMD_INIT);
    _nanosleep();

    lcd_write_date(lcd_dev, LCD_CMD_RETURN_HOME);

    lcd_write_date(lcd_dev,
        (LCD_CMD_FUNCTION_SET & ~(BUS_TYPE_8BIT)) |
        DISPLAT_2_LINE |
        CHAR_5X10_TYPE);

    lcd_write_date(lcd_dev, LCD_CMD_CLEAR_DISP);

    lcd_write_date(lcd_dev, LCD_CMD_DISP_CTRL |
        DISPLAY_ON |
        CURSOR_ON |
        CURSOR_BLINK_ON);

    lcd_write_date(lcd_dev, 0x06);

    return 0;
}

void show_key()
{
    lcd_backlight(&lcd_dev, LCD_BL_BIT_EN);
    lcd_rw(&lcd_dev, LCD_RW_BIT_W);

    lcd_rs(&lcd_dev, RS_BIT_INST);
    lcd_write_date(&lcd_dev, LCD_CMD_RETURN_HOME);
    lcd_write_date(&lcd_dev, LCD_CMD_CLEAR_DISP);

    lcd_rs(&lcd_dev, RS_BIT_DATA);
    lcd_write_date(&lcd_dev, 'K');
    lcd_write_date(&lcd_dev, 'E');
    lcd_write_date(&lcd_dev, 'Y');
    lcd_write_date(&lcd_dev, '0');
    lcd_write_date(&lcd_dev, ':');

    lcd_rs(&lcd_dev, RS_BIT_INST);
    lcd_write_date(&lcd_dev, DISP_LOCATE_2SED);
    lcd_rs(&lcd_dev, RS_BIT_DATA);

    lcd_write_date(&lcd_dev, 'K');
    lcd_write_date(&lcd_dev, 'E');
    lcd_write_date(&lcd_dev, 'Y');
    lcd_write_date(&lcd_dev, '1');
    lcd_write_date(&lcd_dev, ':');

    lcd_rs(&lcd_dev, RS_BIT_INST);
    lcd_write_date(&lcd_dev, DISP_LOCATE_3TH);
    lcd_rs(&lcd_dev, RS_BIT_DATA);

    lcd_write_date(&lcd_dev, 'K');
    lcd_write_date(&lcd_dev, 'E');
    lcd_write_date(&lcd_dev, 'Y');
    lcd_write_date(&lcd_dev, '2');
    lcd_write_date(&lcd_dev, ':');

    lcd_rs(&lcd_dev, RS_BIT_INST);
    lcd_write_date(&lcd_dev, DISP_LOCATE_4TH);
    lcd_rs(&lcd_dev, RS_BIT_DATA);

    lcd_write_date(&lcd_dev, 'K');
    lcd_write_date(&lcd_dev, 'E');
    lcd_write_date(&lcd_dev, 'Y');
    lcd_write_date(&lcd_dev, '3');
    lcd_write_date(&lcd_dev, ':');
}

int main(int argc, char** argv)
{
    int option_index = 0;
    int opts;
    unsigned char w_cmd = 0;
    unsigned char cmd = 0;

    /* getopt_long stores the option index here. */
    struct option long_options[] = {
        {"start", no_argument, 0, 's'},
        {"instr", required_argument, 0, 'i'},
        {"write", required_argument, 0, 'w'},
        {"read", no_argument, 0, 'r'},
        {"test", no_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while (1) {
        opts = getopt_long (argc, argv, "si:w:rt",
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

        case 'w':
            cmd = cmd | WRITE_DATA_MASK;
            w_cmd = strtol(optarg, NULL, 16);
            break;

        case 'r':
            cmd = cmd | READ_DATA_MASK;
            break;
    
        case 't':
            cmd = cmd | TEST_MASK;
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
        lcd_backlight(&lcd_dev, LCD_BL_BIT_EN);
        lcd_rs(&lcd_dev, RS_BIT_INST);
        lcd_rw(&lcd_dev, LCD_RW_BIT_W);
        lcd_write_date(&lcd_dev, w_cmd);
        break;

    case WRITE_DATA_MASK:
        fprintf(stderr, "Writing data 0x%x\n", w_cmd);
        lcd_backlight(&lcd_dev, LCD_BL_BIT_EN);
        lcd_rs(&lcd_dev, RS_BIT_DATA);
        lcd_rw(&lcd_dev, LCD_RW_BIT_W);
        lcd_write_date(&lcd_dev, w_cmd);
        break;

    case READ_DATA_MASK:
        printf("READ_DATA_MASK\r\n");
        lcd_backlight(&lcd_dev, LCD_BL_BIT_EN);
        lcd_rs(&lcd_dev, RS_BIT_DATA);
        lcd_rw(&lcd_dev, LCD_RW_BIT_R);
        lcd_write_date(&lcd_dev, 0x01);

        printf("0x%x\r\n", read_data(&lcd_dev));
        break;

    case TEST_MASK:
        printf("test\r\n");
        show_key();
        break;

    default:
        break;
    }


    lcd_close(&lcd_dev);

    return 0;
}
