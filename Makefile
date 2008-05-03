TARGET := ethersex
TOPDIR := .

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
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)

menuconfig:
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig config.in

##############################################################################
clean:
	$(RM) $(TARGET) $(TARGET).bin $(TARGET).hex *.[oda] pinning.c .subdirs
	for subdir in `find -type d`; do \
	  test "x$$subdir" != "x." \
	  && test -e $$subdir/Makefile \
	  && make no_deps=t -C $$subdir clean; done

PINNING_FILES=pinning/header.m4 pinning/generic.m4 pinning/$(MCU).m4 pinning/footer.m4
pinning.c: $(PINNING_FILES) autoconf.h
	m4 `grep -e "^#define .*_SUPPORT" autoconf.h | sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT.*//"` $(PINNING_FILES) > $@


##############################################################################
# generate SUBDIRS variable
#
ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)

.subdirs: .config
	$(RM) -f $@
	for subdir in `grep -e "^#define .*_SUPPORT" autoconf.h | sed -e "s/^#define //" -e "s/_SUPPORT.*//" | tr "[A-Z]\\n" "[a-z] " ` uip net ; do \
	  test -d $$subdir && echo "SUBDIRS += $$subdir" >> $@; \
	done
include $(TOPDIR)/.subdirs

endif # MAKECMDGOALS=clean
endif # no_deps=t


##############################################################################
# configure ethersex
#
show-config: autoconf.h
	@echo
	@echo "These modules are currently enabled: "
	@echo "======================================"
	@grep -e "^#define .*_SUPPORT" autoconf.h | sed -e "s/^#define / * /" -e "s/_SUPPORT.*//"

.PHONY: show-config

.config: 
ifneq ($(MAKECMDGOALS),menuconfig)  
	# make sure menuconfig isn't called twice, on `make menuconfig'
	$(MAKE) no_deps=t menuconfig
	# test the target file, test fails if it doesn't exist
	# and will keep make from looping menuconfig.
	test -e $@
endif

include depend.mk
