TARGET := ethersex
TOPDIR = .

SUBDIRS += control6
SUBDIRS += core
SUBDIRS += core/crypto
SUBDIRS += core/portio
SUBDIRS += core/tty
SUBDIRS += core/vfs
SUBDIRS += mcuf
SUBDIRS += hardware/adc
SUBDIRS += hardware/adc/kty
SUBDIRS += hardware/clock/dcf77
SUBDIRS += hardware/camera
SUBDIRS += hardware/ethernet
SUBDIRS += hardware/i2c/master
SUBDIRS += hardware/i2c/slave
SUBDIRS += hardware/input/ps2
SUBDIRS += hardware/input/buttons
SUBDIRS += hardware/io_expander
SUBDIRS += hardware/ir/rc5
SUBDIRS += hardware/lcd
SUBDIRS += hardware/lcd/s1d15g10
SUBDIRS += hardware/onewire
SUBDIRS += hardware/pwm
SUBDIRS += hardware/radio/fs20
SUBDIRS += hardware/radio/rfm12
SUBDIRS += hardware/storage/dataflash
SUBDIRS += hardware/storage/sd_reader
SUBDIRS += protocols/bootp
SUBDIRS += protocols/dmx
SUBDIRS += protocols/mdns_sd
SUBDIRS += protocols/modbus
SUBDIRS += protocols/mysql
SUBDIRS += protocols/smtp
SUBDIRS += protocols/snmp
SUBDIRS += protocols/syslog
SUBDIRS += protocols/uip
SUBDIRS += protocols/uip/ipchair
SUBDIRS += protocols/usb
SUBDIRS += protocols/yport
SUBDIRS += protocols/zbus
SUBDIRS += protocols/dns
SUBDIRS += protocols/ecmd/
SUBDIRS += protocols/ecmd/sender
SUBDIRS += protocols/ecmd/via_i2c
SUBDIRS += protocols/ecmd/via_tcp
SUBDIRS += protocols/ecmd/via_udp
SUBDIRS += protocols/ecmd/via_usart
SUBDIRS += protocols/irc
SUBDIRS += protocols/twitter
SUBDIRS += protocols/netstat
SUBDIRS += services/clock
SUBDIRS += services/cron
SUBDIRS += services/dyndns
SUBDIRS += services/echo
SUBDIRS += services/httpd
SUBDIRS += services/jabber
SUBDIRS += services/ntp
SUBDIRS += services/stella
SUBDIRS += services/tftp
SUBDIRS += services/upnp
SUBDIRS += services/watchcat

rootbuild=t

export TOPDIR

##############################################################################
all: compile-$(TARGET)
	@echo "=======The ethersex project========"
	@echo "Compiled for: $(MCU) at $(FREQ)Hz"
	@${TOPDIR}/scripts/size $(TARGET) $(MCU)
	@echo "==================================="
.PHONY: all
.SILENT: all

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/defaults.mk
#include $(TOPDIR)/scripts/rules.mk

ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),fullclean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)

include $(TOPDIR)/.config

endif # MAKECMDGOALS!=menuconfig
endif # MAKECMDGOALS!=fullclean
endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean
endif # no_deps!=t

SRC += ethersex.c
${UIP_SUPPORT}_SRC += network.c

include $(foreach subdir,$(SUBDIRS),$(subdir)/Makefile)

debug:
	@echo SRC: ${SRC}
	@echo y_SRC: ${y_SRC}
	@echo y_ECMD_SRC: ${y_ECMD_SRC}

meta.m4: ${SRC} ${y_SRC} .config
	@sed -ne '/Ethersex META/{n;:loop p;n;/\*\//!bloop }' ${SRC} ${y_SRC} > $@
	@echo "Build meta files"

meta.c: scripts/meta_magic.m4 protocols/ecmd/ecmd_magic.m4 meta.m4 protocols/ecmd/ecmd_defs.m4
	@m4 $^ > $@

##############################################################################

compile-$(TARGET): $(TARGET).hex $(TARGET).bin
.PHONY: compile-$(TARGET)
.SILENT: compile-$(TARGET)

${ECMD_PARSER_SUPPORT}_SRC += ${y_ECMD_SRC}

OBJECTS += $(patsubst %.c,%.o,${SRC} ${y_SRC} meta.c)
OBJECTS += $(patsubst %.S,%.o,${ASRC} ${y_ASRC})

# FIXME how can we omit specifying every file to be linked twice?
# This is currently necessary because of interdependencies between
# the libraries, which aren't denoted in these however.
$(TARGET): $(OBJECTS)
	@$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "Link binary $@."

##############################################################################

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@
.SILENT: %.hex

##############################################################################
#
# VFS Inline Files
#
##############################################################################

ifeq ($(VFS_INLINE_SUPPORT),y)
INLINE_FILES := $(shell ls embed/* | sed '/\.tmp$$/d; /\.gz$$/d; s/\.cpp$$//; s/\.m4$$//; s/\.sh$$//;')
ifeq ($(DEBUG_INLINE_FILES),y)
.PRECIOUS = $(INLINE_FILES)
endif
else
INLINE_FILES :=
endif

embed/%: embed/%.cpp
	@if ! avr-cpp -DF_CPU=$(FREQ) -I$(TOPDIR) $< 2> /dev/null > $@.tmp; \
		then $(RM) $@; echo "--> Don't include $@ ($<)"; \
	else sed '/^$$/d; /^#[^#]/d' <$@.tmp > $@; \
	  echo "--> Include $@ ($<)"; fi
	@$(RM) $@.tmp


embed/%: embed/%.m4
	@if ! m4 `grep -e "^#define .*_SUPPORT" autoconf.h | \
		sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT.*//"` \
		`grep -e "^#define CONF_.*" autoconf.h |  sed -e "s/^#define CONF_/-Dvalue_/" -re "s/( )/=/" -e "s/[ \"]//g"` \
		$< > $@; then $(RM) $@; echo "--> Don't include $@ ($<)";\
		else echo "--> Include $@ ($<)";	fi

embed/%: embed/%.sh
	@if ! $(CONFIG_SHELL) $< > $@; then $(RM) $@; echo "--> Don't include $@ ($<)"; \
		else echo "--> Include $@ ($<)";	fi


%.bin: % $(INLINE_FILES)
	@$(OBJCOPY) -O binary -R .eeprom $< $@
ifeq ($(VFS_INLINE_SUPPORT),y)
	@$(MAKE) -C core/vfs vfs-concat TOPDIR=../.. no_deps=t
	@core/vfs/do-embed $(INLINE_FILES)
	@$(OBJCOPY) -O ihex -I binary $(TARGET).bin $(TARGET).hex
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
		$(patsubst %.o,%.dep,${OBJECTS}) \
		$(patsubst %.o,%.E,${OBJECTS}) \
		$(patsubst %.o,%.s,${OBJECTS}) network.dep
	echo "Cleaning completed"

fullclean: clean
	find $(TOPDIR)/ -type f \( -name '*.o' -o -name '*.dep' \) -print0 \
	| xargs -0 $(RM)
	echo "Full cleaning completed"
	
mrproper: fullclean
	$(RM) autoconf.h .config config.mk .menuconfig.log .config.old
	echo "All object files and config files are gone now"

.PHONY: clean fullclean mrproper
.SILENT: clean fullclean mrproper

##############################################################################
# MCU specific pinning code generation
#
PINNING_FILES=pinning/internals/header.m4 \
	$(wildcard pinning/*/$(MCU).m4) \
	$(wildcard pinning/internals/hackery_$(MCU).m4) \
	$(wildcard pinning/hardware/$(HARDWARE).m4) pinning/internals/footer.m4
pinning.c: $(PINNING_FILES) autoconf.h
	@m4 -I$(TOPDIR)/pinning `grep -e "^#define .*_SUPPORT" autoconf.h | \
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

include $(TOPDIR)/scripts/depend.mk
