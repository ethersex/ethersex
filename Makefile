TARGET := ethersex
TOPDIR = .

SRC = \
	enc28j60.c \
	ethersex.c \
	ipv6.c \
	network.c

##SUBDIRS += aliascmd
SUBDIRS += control6
SUBDIRS += core
SUBDIRS += core/portio
SUBDIRS += crypto
SUBDIRS += dcf77
SUBDIRS += ecmd_parser
SUBDIRS += ecmd_serial
SUBDIRS += hc165
SUBDIRS += hc595
SUBDIRS += i2c_master
SUBDIRS += i2c_slave
SUBDIRS += ipchair
SUBDIRS += kty
SUBDIRS += lcd
SUBDIRS += mcuf
SUBDIRS += mdns_sd
SUBDIRS += net
SUBDIRS += onewire
SUBDIRS += ps2
#SUBDIRS += pwm
SUBDIRS += rc5
SUBDIRS += hardware/camera
SUBDIRS += hardware/radio/fs20
SUBDIRS += hardware/radio/rfm12
SUBDIRS += hardware/storage/dataflash
#SUBDIRS += sd_reader
SUBDIRS += services/dyndns
SUBDIRS += services/clock
SUBDIRS += services/httpd
SUBDIRS += services/ntp
SUBDIRS += services/cron
SUBDIRS += services/stella
SUBDIRS += services/tftp
SUBDIRS += services/jabber
SUBDIRS += services/dns
SUBDIRS += uip
SUBDIRS += protocols/modbus
SUBDIRS += protocols/mysql
SUBDIRS += protocols/snmp
SUBDIRS += protocols/bootp
SUBDIRS += protocols/usb
SUBDIRS += protocols/yport
SUBDIRS += protocols/zbus
SUBDIRS += syslog
SUBDIRS += vfs
SUBDIRS += watchcat

rootbuild=t

export TOPDIR

##############################################################################
all: compile-$(TARGET)
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo "size is: "
	@${TOPDIR}/scripts/size $(TARGET) $(MCU)
	@echo "==============================="


##############################################################################
# generic fluff
include defaults.mk
#include $(TOPDIR)/rules.mk

ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)

include $(TOPDIR)/.config

endif # MAKECMDGOALS!=menuconfig
endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean
endif # no_deps!=t

include $(foreach subdir,$(SUBDIRS),$(subdir)/Makefile)

debug:
	@echo SRC: ${SRC}
	@echo y_SRC: ${y_SRC}

##############################################################################

.PHONY: compile-$(TARGET)
compile-$(TARGET): $(TARGET).hex $(TARGET).bin

OBJECTS += $(patsubst %.c,%.o,${SRC} ${y_SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC} ${y_ASRC})

# FIXME how can we omit specifying every file to be linked twice?
# This is currently necessary because of interdependencies between
# the libraries, which aren't denoted in these however.
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

##############################################################################

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

##############################################################################
#
# VFS Inline Files
#
##############################################################################

ifeq ($(VFS_INLINE_SUPPORT),y)
INLINE_FILES := $(shell ls vfs/embed/* | sed '/\.tmp$$/d; /\.gz$$/d; s/\.cpp$$//; s/\.m4$$//; s/\.sh$$//;')
else
INLINE_FILES :=
endif

vfs/embed/%: vfs/embed/%.cpp
	@if ! avr-cpp -DF_CPU=$(FREQ) $< 2> /dev/null > $@.tmp; \
		then $(RM) -f $@; echo "--> Don't include $@ ($<)"; \
	else sed '/^$$/d; /^#[^#]/d' <$@.tmp > $@; \
	  echo "--> Include $@ ($<)"; fi
	@$(RM) -f $@.tmp


vfs/embed/%: vfs/embed/%.m4
	@if ! m4 `grep -e "^#define .*_SUPPORT" autoconf.h | \
		sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT.*//"` \
		`grep -e "^#define CONF_.*" autoconf.h |  sed -e "s/^#define CONF_/-Dvalue_/" -re "s/( )/=/" -e "s/[ \"]//g"` \
		$< > $@; then $(RM) -f $@; echo "--> Don't include $@ ($<)";\
		else echo "--> Include $@ ($<)";	fi

vfs/embed/%: vfs/embed/%.sh
	@if ! $(CONFIG_SHELL) $< > $@; then $(RM) -f $@; echo "--> Don't include $@ ($<)"; \
		else echo "--> Include $@ ($<)";	fi


%.bin: % $(INLINE_FILES)
	$(OBJCOPY) -O binary -R .eeprom $< $@
ifeq ($(VFS_INLINE_SUPPORT),y)
	$(MAKE) -C vfs vfs-concat TOPDIR=.. no_deps=t
	vfs/do-embed $(INLINE_FILES)
	$(OBJCOPY) -O ihex -I binary $(TARGET).bin $(TARGET).hex
endif

##############################################################################

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<


##############################################################################
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          elif [ -x /bin/bash ]; then echo /bin/bash; \
          elif [ -x /usr/local/bin/bash ]; then echo /usr/local/bin/bash; \
          else echo sh; fi)

menuconfig:
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig config.in
	test -e .config
	@$(MAKE) no_deps=t what-now-msg

what-now-msg:
	@echo ""
	@echo "Next, you can: "
	@echo " * '$(MAKE)' to compile Ethersex"
	@for subdir in $(SUBDIRS); do \
	  test -e "$$subdir/configure" -a -e "$$subdir/cfgpp" \
	    && echo " * '$(MAKE) $$subdir/menuconfig' to" \
	            "further configure $$subdir"; done || true
	@echo ""
.PHONY: what-now-msg

%/menuconfig:
	$(SH) "$(@D)/configure"
	@$(MAKE) what-now-msg

##############################################################################
clean:
	$(RM) $(TARGET) $(TARGET).lss $(TARGET).bin $(TARGET).hex pinning.c
	$(RM) $(OBJECTS) $(CLEAN_FILES) \
		$(patsubst %.o,%.d,${OBJECTS}) \
		$(patsubst %.o,%.E,${OBJECTS}) \
		$(patsubst %.o,%.s,${OBJECTS})

mrproper:
	$(MAKE) clean
	$(RM) -f autoconf.h .config config.mk .menuconfig.log .config.old

.PHONY: clean mrproper


##############################################################################
# MCU specific pinning code generation
#
PINNING_FILES=pinning/internals/header.m4 \
	$(wildcard pinning/*/$(MCU).m4) \
	$(wildcard pinning/internals/hackery_$(MCU).m4) \
	$(wildcard pinning/hardware/$(HARDWARE).m4) pinning/internals/footer.m4
pinning.c: $(PINNING_FILES) autoconf.h
	m4 -I$(TOPDIR)/pinning `grep -e "^#define .*_SUPPORT" autoconf.h | \
	  sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT.*//"` $(PINNING_FILES) > $@


##############################################################################
# configure ethersex
#
show-config: autoconf.h
	@echo
	@echo MCU: $(MCU) Hardware: $(HARDWARE)
	@echo
	@echo "These modules are currently enabled: "
	@echo "======================================"
	@grep -e "^#define .*_SUPPORT" autoconf.h | sed -e "s/^#define / * /" -e "s/_SUPPORT.*//"

.PHONY: show-config

autoconf.h .config:
	@echo $(MAKE)\'s goal: $(MAKECMDGOALS)
ifneq ($(MAKECMDGOALS),menuconfig)
	# make sure menuconfig isn't called twice, on `make menuconfig'
	test -s autoconf.h -a -s .config || $(MAKE) no_deps=t menuconfig
	# test the target file, test fails if it doesn't exist
	# and will keep make from looping menuconfig.
	test -s autoconf.h -a -s .config
	# now let's restart make so the .config is (re)evaluated.
	$(MAKE) $(MAKECMDGOALS)
	@echo Ethersex compiled successfully, ignore make error!
	@false # stop compilation
endif

include depend.mk
