TOPDIR ?= ..
include $(TOPDIR)/.config

$(ADC_SUPPORT)_SRC += ecmd_parser/adc.c
$(KTY_SUPPORT)_SRC += ecmd_parser/kty.c
$(ECMD_PARSER_SUPPORT)_SRC += ecmd_parser/ecmd.c
$(MCUF_SUPPORT)_SRC += ecmd_parser/mcuf.c
$(UIP_SUPPORT)_SRC += ecmd_parser/network.c
$(PORTIO_SUPPORT)_SRC += ecmd_parser/portio.c
$(PORTIO_SIMPLE_SUPPORT)_SRC += ecmd_parser/portio.c
$(I2C_SUPPORT)_SRC += ecmd_parser/i2c.c
$(PWM_SUPPORT)_SRC += ecmd_parser/pwm.c
$(ALIASCMD_SUPPORT)_SRC += ecmd_parser/aliascmd.c
$(ECMD_SCRIPT_SUPPORT)_SRC += ecmd_parser/scripting.c

# syslog.c disabled due to license issues.

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
named_pin_simple_files=../named_pin/np_simple.m4 np_simple_glue.m4
endif
endif

ecmd_parser/ecmd.c: ecmd_parser/ecmd_defs.c
ecmd_parser/ecmd_defs.c: ecmd_parser/ecmd_magic.m4 ecmd_parser/ecmd_defs.m4 $(named_pin_simple_files)
	m4 $^ > $@.tmp
	mv -f $@.tmp $@

ecmd_parser/np_simple_glue.m4: named_pin/config named_pin/np_simple_glue.awk
	grep -v -e "^#" < $< | gawk -f ../named_pin/np_simple_glue.awk > $@

wiki.txt: wiki.m4 ecmd_defs.m4
	m4 $^ | perl -ne 'unless(m/^\s*$$/){s/^\s+//;s/\s+$$//;print "$$_\n"}' > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
CLEAN_FILES += ecmd_parser/ecmd_defs.c ecmd_parser/np_simple_glue.m4 ecmd_parser/wiki.txt
