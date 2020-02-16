CFLAGS = -Wall -O2 -static
CC = $(CROSS_COMPILE)gcc
INSTALL = install
TARGET = $(lcd_test)


lcd_test: lcd.o lcd_2004a.o i2c_if.o
	$(CC) $(CFLAGS) $^ -o $@


install: $(TARGET)
	scp lcd_test $(INSTALLDIR)

clean distclean:
	rm -rf *.o lcd_test

.PHONY: $(PHONY) install clean distclean
