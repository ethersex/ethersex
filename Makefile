TARGET = etherrape

# export current directory to use in sub-makefiles
export PWD

# include avr-generic makefile configuration
include $(PWD)/avr.mk

SRC = $(shell echo *.c net/*.c lcd/*.c)
#SRC += $(shell echo fs20/*.c)
SRC += $(shell echo ecmd/*.c)
#SRC += $(shell echo tetrirape/*.c)
#SRC += $(shell echo bootp/*.c)
#SRC += $(shell echo watchcat/*.c)
#SRC += $(shell echo named_pin/*.c)
#SRC += $(shell echo tftp/*.c)
#SRC += $(shell echo crypto/*.c)
#SRC += $(shell echo onewire/*.c)
#SRC += $(shell echo rc5/*.c)
#SRC += $(shell echo dns/*.c)
#SRC += $(shell echo dyndns/*.c)
#SRC += $(shell echo syslog/*.c)
#SRC += $(shell echo i2c/*.c)
#SRC += uip/uip_openvpn.c                               # OPENVPN_SUPPORT
SRC += uip/uip.c uip/uip_arp.c uip/uip_neighbor.c uip/psock.c uip/uip_multi.c

# preprocessed config files
USER_CONFIG = $(patsubst %/cfgpp,%/user_config.h,$(shell echo */cfgpp))

OBJECTS += $(patsubst %.c,%.o,${SRC})
#CFLAGS += -Werror
#CFLAGS += -Iuip/ -Iuip/apps
#LDFLAGS += -L/usr/local/avr/avr/lib

#LDFLAGS += -Wl,--section-start=.text=0xE000		# BOOTLOADER_SUPPORT
#CFLAGS  += -mcall-prologues                            # BOOTLOADER_SUPPORT

# no safe mode checks, since the bootloader doesn't support this
AVRDUDE_FLAGS += -u
# no signature byte check, -> bootloader bug
AVRDUDE_FLAGS += -F

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
%/% %/%.o %/%.hex %/all %/install:
	$(MAKE) -C $(@D) -e $(@F)

%/clean:
	$(MAKE) -C $(@D) -e $(@F) no_deps=t

# Sorry for this workaround
%.o: %.c
	@ if [ -x $(@D)/cfgpp -a $(@D)/config -nt $(@D)/user_config.h ]; then \
	 	echo "Wrote $(@D)/user_config.h"; \
	 	$(@D)/cfgpp > $(@D)/user_config.h; \
	  fi
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: install

install: program-serial-$(TARGET)

install-eeprom: program-serial-eeprom-$(TARGET)


.PHONY: clean clean-$(TARGET) distclean

clean: clean-$(TARGET) uip/clean net/clean
clean: bootloader/clean fs20/clean lcd/clean onewire/clean
clean: rc5/clean

clean-$(TARGET):
	rm -f $(TARGET) $(TARGET).map $(TARGET).bin
	rm -f $(OBJECTS) $(USER_CONFIG)

distclean: clean
	rm -f eeprom-default.raw Makefile.dep tags

-include depend.mk
