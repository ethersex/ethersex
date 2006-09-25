
TARGET = etherrape

# microcontroller and project specific settings (everything can be overridden using config.mk)
#F_CPU = 16000000UL
#MCU = atmega88

# include avr-generic makefile configuration
include $(CURDIR)/avr.mk

SRC = $(shell echo *.c)
OBJECTS += $(patsubst %.c,%.o,${SRC})
#CFLAGS += -Werror
#CFLAGS += -Iuip/ -Iuip/apps
#LDFLAGS += -L/usr/local/avr/avr/lib

# no safe mode checks, since the bootloader doesn't support this
AVRDUDE_FLAGS += -u

# Name of Makefile for make depend
MAKEFILE = Makefile

# export current directory to use in sub-makefiles
export CURDIR

.PHONY: all

all: $(TARGET).hex $(TARGET).eep.hex $(TARGET).lss
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET): $(OBJECTS) $(TARGET).o

$(OBJECTS): config.mk

# subdir magic

%/% %/%.o %/%.hex %/all %/depend %/install:
	$(MAKE) -C $(@D) -e $(@F)



.PHONY: install

install: program-serial-$(TARGET)

install-eeprom: program-serial-eeprom-$(TARGET)


.PHONY: clean clean-$(TARGET)

clean: clean-$(TARGET)

clean-$(TARGET):
	rm -f $(TARGET)
	rm -f $(OBJECTS)

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE).dep

%/depend:
	$(MAKE) -C $(@D) -e depend

-include $(MAKEFILE).dep
