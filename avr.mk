# etherrape project specific defaults, each can be overridden in config.mk
F_CPU = 20000000UL
MCU = atmega644
AVRDUDE_BAUDRATE=115200
UART_BAUDRATE=115200


# Programmer used for In System Programming
ISP_PROG = dapa
# device the ISP programmer is connected to
ISP_DEV = /dev/parport0
# Programmer used for serial programming (using the bootloader)
SERIAL_PROG = avr109
# device the serial programmer is connected to
SERIAL_DEV = /dev/ttyS0

# programs
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 115200
SIZE = avr-size

all:

$(PWD)/config.mk:
	@echo "# Put your own config here!" > $@
	@echo "#SERIAL_DEV = $(SERIAL_DEV)" >> $@
	@echo "#DEBUG = 1\n" >> $@
	@echo "created default config.mk, tune your settings there!"

-include $(PWD)/config.mk

# flags for avrdude
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
endif
ifeq ($(MCU),atmega88)
	AVRDUDE_MCU=m88
endif
ifeq ($(MCU),atmega168)
	AVRDUDE_MCU=m168
endif
ifeq ($(MCU),atmega32)
	AVRDUDE_MCU=m32
endif
ifeq ($(MCU),atmega644)
	AVRDUDE_MCU=m644
endif

AVRDUDE_FLAGS += -p $(AVRDUDE_MCU) -b $(AVRDUDE_BAUDRATE)

# flags for the compiler
CFLAGS += -g -Os -finline-limit=800 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99

# flags for the linker
LDFLAGS += -mmcu=$(MCU)

DEBUG_CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
DEBUG_CFLAGS += -Wmissing-declarations -Wredundant-decls
DEBUG_CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
DEBUG_CFLAGS += -Winline -Wpointer-arith -Wsign-compare
DEBUG_CFLAGS += -Wunreachable-code -Wdisabled-optimization
DEBUG_CFLAGS += -fshort-enums
DEBUG_CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
DEBUG_CFLAGS += -Wa,-adhlns=$(basename $@).lst
DEBUG_CFLAGS += -DDEBUG

ifneq ($(DEBUG),)
	CFLAGS += $(DEBUG_CFLAGS)
endif


.PHONY: sanity-check

sanity-check:
ifndef F_CPU
	@echo "please define F_CPU!"
	@exit 1
endif

ifndef MCU
	@echo "please define MCU!"
	@exit 1
endif


$(OBJECTS): sanity-check

clean:
	$(RM) *.hex *.eep.hex *.o *.lst *.lss

interactive-isp:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -t

interactive-serial:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(SERIAL_PROG) -P $(SERIAL_DEV) -t


.PHONY: all clean interactive-isp interactive-serial launch-bootloader

program-isp-%: %.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U flash:w:$<

program-isp-eeprom-%: %.eep.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U eeprom:w:$<

program-serial-%: %.hex launch-bootloader
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U flash:w:$<

program-serial-eeprom-%: %.eep.hex launch-bootloader
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U eeprom:w:$<

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<

launch-bootloader: bootloader/launcher/launch-bootloader
	$^ $(SERIAL_DEV) $(AVRDUDE_BAUDRATE)

bootloader/launcher/launch-bootloader:
	$(MAKE) -C bootloader/launcher launch-bootloader
