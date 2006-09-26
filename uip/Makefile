# include avr-generic makefile configuration

include $(CURDIR)/avr.mk

SRC = $(shell echo *.c)
OBJECTS += $(patsubst %.c,%.o,${SRC})

# Name of Makefile for make depend
MAKEFILE = Makefile

.PHONY: all

DEBUG = 0

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE).dep

-include $(MAKEFILE).dep

