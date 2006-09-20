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

-include config.mk

# flags for avrdude
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
else ifeq ($(MCU),atmega88)
	AVRDUDE_MCU=m88
else ifeq ($(MCU),atmega168)
	AVRDUDE_MCU=m168
else ifeq ($(MCU),atmega32)
	AVRDUDE_MCU=m32
else ifeq ($(MCU),atmega644)
	AVRDUDE_MCU=m644
endif

AVRDUDE_FLAGS += -p $(AVRDUDE_MCU) -b $(AVRDUDE_BAUDRATE)

# flags for the compiler
CFLAGS += -g -Os -finline-limit=800 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99

# flags for the linker
LDFLAGS += -mmcu=$(MCU)

ifneq ($(DEBUG),)
	CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS += -Wmissing-declarations -Wredundant-decls
	CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS += -Winline -Wpointer-arith -Wsign-compare
	CFLAGS += -Wunreachable-code -Wdisabled-optimization -Werror
	CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS += -Wa,-adhlns=$(basename $@).lst
	CFLAGS += -DDEBUG
endif

all:

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


.PHONY: all clean interactive-isp interactive-serial

program-isp-%: %.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U flash:w:$<

program-isp-eeprom-%: %.eep.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U eeprom:w:$<

program-serial-%: %.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U flash:w:$<

program-serial-eeprom-%: %.eep.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U eeprom:w:$<

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<


