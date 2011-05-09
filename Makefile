TARGET := ethersex
TOPDIR = .

SUBDIRS += control6
SUBDIRS += core
SUBDIRS += core/crypto
SUBDIRS += core/host
SUBDIRS += core/portio
SUBDIRS += core/tty
SUBDIRS += core/gui
SUBDIRS += core/util
SUBDIRS += core/vfs
SUBDIRS += mcuf
SUBDIRS += hardware/adc
SUBDIRS += hardware/adc/kty
SUBDIRS += hardware/adc/ads7822
SUBDIRS += hardware/avr
SUBDIRS += hardware/dac
SUBDIRS += hardware/clock/dcf77
SUBDIRS += hardware/camera
SUBDIRS += hardware/ethernet
SUBDIRS += hardware/i2c/master
SUBDIRS += hardware/i2c/slave
SUBDIRS += hardware/input
SUBDIRS += hardware/input/ps2
SUBDIRS += hardware/input/buttons
SUBDIRS += hardware/io_expander
SUBDIRS += hardware/ir/rc5
SUBDIRS += hardware/ir/irmp
SUBDIRS += hardware/isdn
SUBDIRS += hardware/lcd
SUBDIRS += hardware/lcd/s1d15g10
SUBDIRS += hardware/lcd/ST7626
SUBDIRS += hardware/lcd/s1d13305
SUBDIRS += hardware/onewire
SUBDIRS += hardware/pwm
SUBDIRS += hardware/sms
SUBDIRS += hardware/radio/fs20
SUBDIRS += hardware/radio/rfm12
SUBDIRS += hardware/sht
SUBDIRS += hardware/sram
SUBDIRS += hardware/storage/dataflash
SUBDIRS += hardware/storage/sd_reader
SUBDIRS += hardware/zacwire
SUBDIRS += hardware/ultrasonic
SUBDIRS += hardware/hbridge
SUBDIRS += protocols/artnet
SUBDIRS += protocols/bootp
SUBDIRS += protocols/dhcp 
SUBDIRS += protocols/dmx
SUBDIRS += protocols/fnordlicht
SUBDIRS += protocols/mdns_sd
SUBDIRS += protocols/modbus
SUBDIRS += protocols/mysql
SUBDIRS += protocols/smtp
SUBDIRS += protocols/sms77
SUBDIRS += protocols/snmp
SUBDIRS += protocols/syslog
SUBDIRS += protocols/uip
SUBDIRS += protocols/uip/ipchair
SUBDIRS += protocols/ustream
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
SUBDIRS += protocols/soap
SUBDIRS += protocols/httplog
SUBDIRS += protocols/twitter
SUBDIRS += protocols/netstat
SUBDIRS += protocols/to1
SUBDIRS += protocols/serial_line_log
SUBDIRS += protocols/msr1
SUBDIRS += protocols/nmea
SUBDIRS += protocols/udpIO
SUBDIRS += protocols/udpstella
SUBDIRS += protocols/udpcurtain
SUBDIRS += protocols/cw
SUBDIRS += services/clock
SUBDIRS += services/cron
SUBDIRS += services/dyndns
SUBDIRS += services/echo
SUBDIRS += services/pam
SUBDIRS += services/httpd
SUBDIRS += services/jabber
SUBDIRS += services/ntp
SUBDIRS += services/wol
SUBDIRS += services/motd
SUBDIRS += services/moodlight
SUBDIRS += services/stella
SUBDIRS += services/starburst
SUBDIRS += services/tftp
SUBDIRS += services/upnp
SUBDIRS += services/appsample
SUBDIRS += services/watchcat
SUBDIRS += services/vnc
SUBDIRS += services/watchasync
SUBDIRS += services/curtain
SUBDIRS += services/glcdmenu

rootbuild=t

export TOPDIR

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

##############################################################################
ifeq ($(ARCH_HOST),y)
all: $(TARGET)
else
all: compile-$(TARGET)
	@echo "=======The ethersex project========"
	@echo "Compiled for: $(MCU) at $(FREQ)Hz"
	@$(CONFIG_SHELL) ${TOPDIR}/scripts/size $(TARGET) $(MCU)
	@echo "==================================="
endif
.PHONY: all
.SILENT: all

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/defaults.mk
#include $(TOPDIR)/scripts/rules.mk

SRC += ethersex.c
${UIP_SUPPORT}_SRC += network.c

include $(foreach subdir,$(SUBDIRS),$(subdir)/Makefile)

debug:
	@echo SRC: ${SRC}
	@echo y_SRC: ${y_SRC}
	@echo y_ECMD_SRC: ${y_ECMD_SRC}
	@echo y_SOAP_SRC: ${y_SOAP_SRC}

${ECMD_PARSER_SUPPORT}_SRC += ${y_ECMD_SRC}
${SOAP_SUPPORT}_SRC += ${y_SOAP_SRC}

meta.m4: ${SRC} ${y_SRC} .config
	@echo "Build meta files"
	$(SED) -ne '/Ethersex META/{n;:loop p;n;/\*\//!bloop }' ${SRC} ${y_SRC} > $@.tmp
	@echo "Copying to meta.m4"
	@if [ ! -e $@ ]; then cp $@.tmp $@; fi
	@if ! diff $@.tmp $@ >/dev/null; then cp $@.tmp $@; fi
	@rm -f $@.tmp

$(ECMD_PARSER_SUPPORT)_NP_SIMPLE_META_SRC = protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}
$(SOAP_SUPPORT)_NP_SIMPLE_META_SRC = protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}

y_META_SRC += scripts/meta_magic.m4
$(ECMD_PARSER_SUPPORT)_META_SRC += protocols/ecmd/ecmd_magic.m4
$(SOAP_SUPPORT)_META_SRC += protocols/soap/soap_magic.m4
y_META_SRC += meta.m4
$(ECMD_PARSER_SUPPORT)_META_SRC += protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}
y_META_SRC += $(y_NP_SIMPLE_META_SRC)

meta.c: $(y_META_SRC)
	$(M4) `scripts/m4-defines` $^ > $@

meta.h: scripts/meta_header_magic.m4 meta.m4
	$(M4) `scripts/m4-defines` $^ > $@

##############################################################################


compile-$(TARGET): $(TARGET).hex $(TARGET).bin
.PHONY: compile-$(TARGET)
.SILENT: compile-$(TARGET)

OBJECTS += $(patsubst %.c,%.o,${SRC} ${y_SRC} meta.c)
OBJECTS += $(patsubst %.S,%.o,${ASRC} ${y_ASRC})

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) -lc -lm # Pixie Dust!!! (Bug in avr-binutils)

SIZEFUNCARG ?= -e printf -e scanf -e divmod
size-check: $(OBJECTS) ethersex
	@for obj in $^; do \
	    if avr-nm $$obj | grep -q $(SIZEFUNCARG); then \
		echo -n "$$obj: "; avr-nm $$obj | grep $(SIZEFUNCARG) | cut -c12- | tr '\n' ','; echo ''; \
	    fi; \
	done

##############################################################################

# Generate ethersex.hex file
# If inlining is enabled, we need to copy from ethersex.bin to not lose
# those files.  However we mustn't always copy the binary, since that way
# a bootloader cannot be built (the section start address would get lost).
ifeq ($(VFS_INLINE_SUPPORT),y)
%.hex: %.bin
	$(OBJCOPY) -O ihex -I binary $(TARGET).bin $(TARGET).hex
else
%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@
endif
.SILENT: %.hex

##############################################################################
#
# VFS Inline Files
#
##############################################################################

ifeq ($(VFS_INLINE_SUPPORT),y)
INLINE_FILES := $(shell ls embed/* | $(SED) '/\.tmp$$/d; /\.gz$$/d; s/\.cpp$$//; s/\.m4$$//; s/\.sh$$//;')
ifeq ($(DEBUG_INLINE_FILES),y)
.PRECIOUS = $(INLINE_FILES)
endif
else
INLINE_FILES :=
endif

embed/%: embed/%.cpp
	@if ! avr-cpp -DF_CPU=$(FREQ) -I$(TOPDIR) -include autoconf.h $< 2> /dev/null > $@.tmp; \
		then $(RM) $@; echo "--> Don't include $@ ($<)"; \
	else $(SED) '/^$$/d; /^#[^#]/d' <$@.tmp > $@; \
	  echo "--> Include $@ ($<)"; fi
	@$(RM) $@.tmp


embed/%: embed/%.m4
	@if ! $(M4) `scripts/m4-defines` $< > $@; \
	  then $(RM) $@; echo "--> Don't include $@ ($<)";\
		else echo "--> Include $@ ($<)";	fi

embed/%: embed/%.sh
	@if ! $(CONFIG_SHELL) $< > $@; then $(RM) $@; echo "--> Don't include $@ ($<)"; \
		else echo "--> Include $@ ($<)";	fi


%.bin: % $(INLINE_FILES)
	$(OBJCOPY) -O binary -R .eeprom $< $@
ifeq ($(VFS_INLINE_SUPPORT),y)
	@$(MAKE) -C core/vfs vfs-concat TOPDIR=../.. no_deps=t
	$(CONFIG_SHELL) core/vfs/do-embed $(INLINE_FILES)
endif

##############################################################################

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<


##############################################################################
### Special case for MacOS X (darwin10.0) and FreeBSD
CONFIG_SHELL := $(shell if [ x"$$OSTYPE" = x"darwin10.0" ] ; then echo /opt/local/bin/bash; \
          elif [ x"$$OSTYPE" = x"FreeBSD" ]; then echo /usr/local/bin/bash; \
          elif [ -x "$$BASH" ]; then echo $$BASH; \
          elif [ -x /bin/bash ]; then echo /bin/bash; \
          elif [ -x /usr/local/bin/bash ]; then echo /usr/local/bin/bash; \
          else echo sh; fi)
### Special case for MacOS X (darwin10.0)
### bash v3.2 in 10.6 does not work, use version 4.0 from macports
### (see "Voraussetzungen" in wiki)

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
	$(RM) meta.c meta.h meta.m4
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
	$(M4) -I$(TOPDIR)/pinning `scripts/m4-defines` $(PINNING_FILES) > $@


##############################################################################
# configure ethersex
#
show-config: autoconf.h
	@echo
	@echo MCU: $(MCU) Hardware: $(HARDWARE)
	@echo
	@echo "These modules are currently enabled: "
	@echo "======================================"
	@$(SED) -e "/^#define \<.*_SUPPORT\>/!d;s/^#define / * /;s/_SUPPORT.*//" autoconf.h 

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
