TARGET = onewiretest

include $(CURDIR)/avr.mk

SRC = $(shell echo *.c) ../fifo.c ../uart.c ../crc.c
OBJECTS += $(patsubst %.c,%.o,${SRC})
HEADERS += $(shell echo *.h)
#CFLAGS += -Werror
LDFLAGS += -L/usr/local/avr/avr/lib

# no safe mode checks, since the bootloader doesn't support this
AVRDUDE_FLAGS += -u

# Name of Makefile for make depend
MAKEFILE = Makefile

.PHONY: all

all: $(TARGET).hex $(TARGET).lss
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $< | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET): $(OBJECTS) $(TARGET).o

%.o: $(HEADERS)

.PHONY: install

install: program-serial-$(TARGET)

.PHONY: clean clean-$(TARGET)

clean: clean-$(TARGET)

clean-$(TARGET):
	rm -f $(TARGET)
	rm -f $(TARGET).lst

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE).dep

-include $(MAKEFILE).dep
