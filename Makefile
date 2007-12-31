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
	spi.c \
	timer.c

SUBDIRS += net

#SUBDIRS += bootp
#SUBDIRS += clock
#SUBDIRS += dcf77
#SUBDIRS += dns
#SUBDIRS += dyndns
#SUBDIRS += fs20
#SUBDIRS += i2c
SUBDIRS += lcd
#SUBDIRS += named_pin
#SUBDIRS += ntp
#SUBDIRS += onewire
#SUBDIRS += rc5
#SUBDIRS += rfm12
#SUBDIRS += stella
#SUBDIRS += syslog
#SUBDIRS += tetrirape
#SUBDIRS += tftp
#SUBDIRS += watchcat
#SUBDIRS += zbus

SUBDIRS += ecmd
SUBDIRS += uip


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

$(TARGET): $(OBJECTS) $(LINKLIBS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) \
	  $(foreach subdir,$(SUBDIRS),-L$(subdir) -l$(subdir))



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
# make ``clean'' recursive
clean: clean-common
	$(RM) $(TARGET) $(TARGET).bin $(TARGET).hex *.[oda]
	for subdir in `find -type d`; do \
	  test "x$$subdir" != "x." \
	  && test -e $$subdir/Makefile \
	  && make no_deps=t -C $$subdir clean; done

include depend.mk
