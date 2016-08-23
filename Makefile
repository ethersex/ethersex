TARGET := ethersex
TOPDIR = .

SUBDIRS += control6
SUBDIRS += core
SUBDIRS += mcuf
SUBDIRS += hardware
SUBDIRS += protocols
SUBDIRS += services

rootbuild=t

export TOPDIR

ifneq ($(no_deps),t)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),fullclean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)
ifneq ($(MAKECMDGOALS),indent)

include $(TOPDIR)/.config

endif # MAKECMDGOALS!=indent
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
	@$(CONFIG_SHELL) ${TOPDIR}/scripts/size $(TARGET) $(MCU) $(BOOTLOADER_SUPPORT) $(BOOTLOADER_SIZE)
	@$(CONFIG_SHELL) ${TOPDIR}/scripts/eeprom-usage "$(CFLAGS)" "$(CPPFLAGS)" 2> /dev/null
	@echo "==================================="
endif
.PHONY: all
.SILENT: all

##############################################################################
# logging to file make.log
# calls make all and redirects stdout and stderr to make.log
v:
	(echo "===== logging make activity to file make.log =====";\
	 echo "Build started on `date`";\
	 ${MAKE} all 2>&1) | tee make.log

##############################################################################
# print information about binary size and flash usage
size-info:
	@echo "===== size info ====="
	@$(CONFIG_SHELL) ${TOPDIR}/scripts/size $(TARGET) $(MCU) $(BOOTLOADER_SUPPORT) $(BOOTLOADER_SIZE)

##############################################################################
# target help displays a short overview over make options
help:
	@echo "Configuration targets:"
	@echo "  menuconfig   - Update current config utilising a menu based program"
	@echo "                 (default when .config does not exist)"
	@echo ""
	@echo "Cleaning targets:"
	@echo "  clean        - Remove bin and dep files"
	@echo "  fullclean    - Same as "clean", but also remove object files"
	@echo "  mrproper     - Same as "fullclean", but also remove all config files"
	@echo ""
	@echo "Information targets:"
	@echo "  show-config  - show enabled modules"
	@echo "  size-info    - show size information of compiled binary"
	@echo ""
	@echo "Other generic targets:"
	@echo "  all          - Build everything as specified in .config"
	@echo "                 (default if .config exists)"
	@echo "  v            - Same as "all" but with logging to make.log enabled"

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/defaults.mk
#include $(TOPDIR)/scripts/rules.mk

SRC += ethersex.c
${UIP_SUPPORT}_SRC += network.c

include $(foreach subdir,$(SUBDIRS),$(subdir)/Makefile)
include $(foreach subdir,$(SUBSUBDIRS),$(subdir)/Makefile)

SUBDIRS += ${SUBSUBDIRS}

debug:
	@echo SRC: ${SRC}
	@echo y_SRC: ${y_SRC}
	@echo y_ECMD_SRC: ${y_ECMD_SRC}
	@echo y_SOAP_SRC: ${y_SOAP_SRC}

${ECMD_PARSER_SUPPORT}_SRC += ${y_ECMD_SRC}
${SOAP_SUPPORT}_SRC += ${y_SOAP_SRC}

$(ECMD_PARSER_SUPPORT)_NP_SIMPLE_META_SRC = protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}
$(SOAP_SUPPORT)_NP_SIMPLE_META_SRC = protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}

ifeq ($(SCHEDULER_SUPPORT), y)
y_META_SRC += scripts/meta_magic_scheduler.m4
else
y_META_SRC += scripts/meta_magic.m4
endif
$(ECMD_PARSER_SUPPORT)_META_SRC += protocols/ecmd/ecmd_magic.m4
$(SOAP_SUPPORT)_META_SRC += protocols/soap/soap_magic.m4
y_META_SRC += meta.m4
$(ECMD_PARSER_SUPPORT)_META_SRC += protocols/ecmd/ecmd_defs.m4 ${named_pin_simple_files}
y_META_SRC += $(y_NP_SIMPLE_META_SRC)

meta.m4: ${SRC} ${y_SRC} .config
	@echo "Build meta files"
	$(SED) -ne '/Ethersex META/{n;:loop p;n;/\*\//!bloop }' ${SRC} ${y_SRC} > $@.tmp
	@echo "Copying to meta.m4"
	@if [ ! -e $@ ]; then cp -v $@.tmp $@; \
		elif ! diff $@.tmp $@ >/dev/null; then cp -v $@.tmp $@; else echo "$@ unaltered"; fi
	@$(RM) -f $@.tmp

meta.defines: autoconf.h pinning.c
	scripts/m4-defines > $@.tmp
	$(SED) -e "/^#define [A-Z].*_PIN /!d" -e "s/^#define \([^ 	]*\)_PIN.*/-Dpin_\1/;s/[()]/_/g" pinning.c >> $@.tmp
	$(SED) -e ':a' -e 'N' -e '$$!ba' -e 's/\n/ /g' $@.tmp > $@.new
	@echo "Copying to meta.defines"
	@if [ ! -e $@ ]; then cp -v $@.new $@; \
		elif ! diff $@.new $@ >/dev/null; then cp -v $@.new $@; else echo "$@ unaltered"; fi
	@$(RM) -f $@.tmp $@.new

meta.c: meta.defines $(y_META_SRC)
	$(M4) $(M4FLAGS) `cat meta.defines` $(filter-out $<,$^) > $@

meta.h: meta.defines scripts/meta_header_magic.m4 meta.m4
	$(M4) `cat meta.defines` $(filter-out $<,$^) > $@

##############################################################################


compile-$(TARGET): $(TARGET).hex $(TARGET).bin
.PHONY: compile-$(TARGET)
.SILENT: compile-$(TARGET)

OBJECTS += $(patsubst %.c,%.o,${SRC} ${y_SRC} meta.c)
OBJECTS += $(patsubst %.c,%.o,${AUTOGEN_SRC} ${y_AUTOGEN_SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC} ${y_ASRC})

# Do not add version.c to SRC or OBJECTS!
# Compile version.c at link time ensures correct built time, ref. issue #448
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c version.c
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) version.o -lm -lc # Pixie Dust!!! (Bug in avr-binutils)

SIZEFUNCARG ?= -e printf -e scanf -e divmod
size-check: $(OBJECTS) ethersex
	@for obj in $^; do \
	    if avr-nm $$obj | grep -q $(SIZEFUNCARG); then \
		echo -n "$$obj: "; avr-nm $$obj | grep $(SIZEFUNCARG) | cut -c12- | tr '\n' ','; echo ''; \
	    fi; \
	done

##############################################################################

# Generate ethersex.hex file
# If inlining or crc-padding is enabled, we need to copy from ethersex.bin to not lose
# those files.  However we mustn't always copy the binary, since that way
# a bootloader cannot be built (the section start address would get lost).
hex_from_bin =
ifeq ($(VFS_INLINE_SUPPORT),y)
	hex_from_bin = yes
endif
ifeq ($(CRC_PAD_SUPPORT),y)
	hex_from_bin = yes
endif

ifeq ($(hex_from_bin), yes)
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
INLINE_FILES := $(shell ls embed/* | $(SED) '/\.tmp$$/d; /\.gz$$/d; /~$$/d; s/\.cpp$$//; s/\.m4$$//; s/\.sh$$//;')
ifeq ($(DEBUG_INLINE_FILES),y)
.PRECIOUS = $(INLINE_FILES)
endif
else
INLINE_FILES :=
endif

# calculate the flash size when padding a crc
ifeq ($(CRC_PAD_SUPPORT),y)
	FLASHEND = $(shell ./core/crc/read-define FLASHEND)
	ifeq ($(BOOTLOADER_SUPPORT),y)
		fillto = $(shell echo $$(( $(BOOTLOADER_SIZE) - 2 )) )
	else
		ifeq ($(BOOTLOADER_JUMP),y)
			fillto = $(shell echo $$(( $(FLASHEND) - $(BOOTLOADER_SIZE) - 1 )) )
		else
			fillto = $(shell echo $$(( $(FLASHEND) - 1 )) )
		endif
	endif
endif

embed/%: embed/%.cpp
	@if ! avr-cpp -xc -P -DF_CPU=$(FREQ) -I$(TOPDIR) -include autoconf.h $< 2> /dev/null > $@.tmp; \
	  then $(RM) $@; echo "--> Don't include $@ ($<)"; \
	  else $(SED) '/^$$/d; /^#[^#]/d' <$@.tmp > $@; echo "--> Include $@ ($<)";  \
	fi
	@$(RM) $@.tmp


embed/%: embed/%.m4
	@if ! $(M4) `scripts/m4-defines` $< > $@; \
	  then $(RM) $@; echo "--> Don't include $@ ($<)"; \
	  else echo "--> Include $@ ($<)"; \
	fi

embed/%: embed/%.sh
	@if ! $(CONFIG_SHELL) $< > $@; \
	  then $(RM) $@; echo "--> Don't include $@ ($<)"; \
	  else echo "--> Include $@ ($<)"; \
	fi

%.bin: % $(INLINE_FILES)
	$(OBJCOPY) -O binary -R .eeprom $< $@
ifeq ($(VFS_INLINE_SUPPORT),y)
	@$(MAKE) -C core/vfs vfs-concat TOPDIR=../.. no_deps=t
	$(CONFIG_SHELL) core/vfs/do-embed $(INLINE_FILES)
endif
ifeq ($(CRC_PAD_SUPPORT),y)
# fill up the binary to the maximum possible size minus 2 bytes for the crc
	$(OBJCOPY) -I binary -O binary --gap-fill 0xFF --pad-to $(fillto) ethersex.bin ethersex.bin
# pad the crc to the binary
	@$(MAKE) -C core/crc crc16-concat TOPDIR=../.. no_deps=t
	./core/crc/crc16-concat ethersex.bin
endif

##############################################################################

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<


##############################################################################
### Special case for MacOS X and FreeBSD
CONFIG_SHELL := $(shell if [ x"`uname`" = x"Darwin" ] && [ -x /opt/local/bin/bash ] ; then echo /opt/local/bin/bash; \
          elif [ x"`uname`" = x"Darwin" ] && [ -x /usr/local/bin/bash ] ; then echo /usr/local/bin/bash; \
          elif [ x"`uname`" = x"FreeBSD" ]; then echo /usr/local/bin/bash; \
          elif [ -x "$$BASH" ]; then echo $$BASH; \
          elif [ -x /bin/bash ]; then echo /bin/bash; \
          elif [ -x /usr/local/bin/bash ]; then echo /usr/local/bin/bash; \
          else echo sh; fi)
### Special case for MacOS X
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
	$(RM) meta.c meta.h meta.m4 meta.defines
	$(RM) $(AUTOGEN_SRC) $(y_AUTOGEN_SRC)
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
	@$(SED) -e "/^#define \<.*_SUPPORT\>/!d;s/^#define / * /;s/_SUPPORT.*//" autoconf.h|sort -u 

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


##############################################################################
# reformat source code
indent: INDENTCMD=indent -nbad -sc -nut -bli0 -blf -cbi0 -cli2 -npcs -nbbo
indent:
	@find . $(SUBDIRS) -maxdepth 1 -name "*.[ch]" | \
	 egrep -v "(ir.*_lib|core/crypto)" | \
	 while read f; do \
	  $(INDENTCMD) "$$f"; \
	done

.PHONY: indent

include $(TOPDIR)/scripts/avrdude.mk

include $(TOPDIR)/scripts/depend.mk
