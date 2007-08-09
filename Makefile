TARGET = etherrape

# export current directory to use in sub-makefiles
export PWD

# include avr-generic makefile configuration
include $(PWD)/avr.mk

SRC = $(shell echo *.c net/*.c fs20/*.c lcd/*.c)
#SRC += $(shell echo ecmd/*.c)
SRC += $(shell echo tetrirape/*.c)
SRC += uip/uip.c uip/uip_arp.c uip/uip_neighbor.c uip/psock.c
OBJECTS += $(patsubst %.c,%.o,${SRC})
#CFLAGS += -Werror
#CFLAGS += -Iuip/ -Iuip/apps
#LDFLAGS += -L/usr/local/avr/avr/lib

# no safe mode checks, since the bootloader doesn't support this
AVRDUDE_FLAGS += -u
# no signature byte check, -> bootloader bug
AVRDUDE_FLAGS += -F

# Name of Makefile for make depend
MAKEFILE = Makefile

# if you would like everything to be recompiled when you change config.mk, uncomment this line:
#$(OBJECTS): config.mk

.PHONY: all

all: $(TARGET).hex $(TARGET).lss
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET): $(OBJECTS) $(TARGET).o

# subdir magic
%/% %/%.o %/%.hex %/all %/depend %/install %/clean:
	$(MAKE) -C $(@D) -e $(@F)

.PHONY: install

install: program-serial-$(TARGET)

install-eeprom: program-serial-eeprom-$(TARGET)


.PHONY: clean clean-$(TARGET) distclean

clean: clean-$(TARGET) uip/clean net/clean bootloader/clean fs20/clean

clean-$(TARGET):
	rm -f $(TARGET) $(TARGET).map
	rm -f $(OBJECTS)

distclean: clean
	rm -f eeprom-default.raw Makefile.dep tags

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) $(ASRC) > $(MAKEFILE).dep

-include $(MAKEFILE).dep
