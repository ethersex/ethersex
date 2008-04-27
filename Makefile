TARGET=ethersex
TOPDIR=.

SRC = \
	debug.c \
	eeprom.c \
	enc28j60.c \
	etherrape.c \
	ipv6.c \
	network.c \
	portio.c \
	usart.c \
	spi.c \
	timer.c


#SUBDIRS += bootp
#SUBDIRS += clock
#SUBDIRS += cron
#SUBDIRS += crypto
#SUBDIRS += dcf77
#SUBDIRS += dns
#SUBDIRS += dyndns
SUBDIRS += ecmd
#SUBDIRS += fs20
#SUBDIRS += i2c
SUBDIRS += lcd
#SUBDIRS += named_pin
SUBDIRS += net
#SUBDIRS += ntp
#SUBDIRS += mdns_sd
#SUBDIRS += onewire
#SUBDIRS += ps2
#SUBDIRS += rc5
#SUBDIRS += rfm12
#SUBDIRS += stella
#SUBDIRS += syslog
#SUBDIRS += tetrirape
#SUBDIRS += tftp
SUBDIRS += uip
#SUBDIRS += watchcat
#SUBDIRS += i2c_slave
#SUBDIRS += hc595
#SUBDIRS += hc165
#SUBDIRS += sensormodul
#SUBDIRS += modbus
#SUBDIRS += zbus
#SUBDIRS += yport


##############################################################################
all: compile-ethersex
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="


##############################################################################
# generic fluff
include defaults.mk
#include $(TOPDIR)/rules.mk
##############################################################################

.PHONY: compile-subdirs
compile-subdirs:
	for dir in $(SUBDIRS); do make -C $$dir lib$$dir.a; done

.PHONY: compile-$(TARGET)
compile-$(TARGET): compile-subdirs $(TARGET).hex $(TARGET).bin

OBJECTS += $(patsubst %.c,%.o,${SRC})
LINKLIBS = $(foreach subdir,$(SUBDIRS),$(subdir)/lib$(subdir).a)

# FIXME how can we omit specifying every file to be linked twice?
# This is currently necessary because of interdependencies between
# the libraries, which aren't denoted in these however.
$(TARGET): $(OBJECTS) $(LINKLIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) \
	  $(foreach subdir,$(SUBDIRS),-L$(subdir) -l$(subdir)) \
	  $(foreach subdir,$(SUBDIRS),-l$(subdir))


##############################################################################

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.bin: %
	$(OBJCOPY) -O binary -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<



##############################################################################
clean:
	$(RM) $(TARGET) $(TARGET).bin $(TARGET).hex *.[oda] pinning.c
	for subdir in `find -type d`; do \
	  test "x$$subdir" != "x." \
	  && test -e $$subdir/Makefile \
	  && make no_deps=t -C $$subdir clean; done

PINNING_FILES=pinning/header.m4 pinning/generic.m4 pinning/$(MCU).m4 pinning/footer.m4
pinning.c: $(PINNING_FILES) config.h
	m4 `grep -e "^#define .*_SUPPORT$$" config.h | sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT//"` $(PINNING_FILES) > $@
	

include depend.mk
