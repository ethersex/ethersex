TOPDIR ?= ..
include $(TOPDIR)/.config

$(ECMD_PARSER_SUPPORT)_SRC += ecmd_parser/ecmd.c
$(MCUF_SUPPORT)_SRC += ecmd_parser/mcuf.c
$(UIP_SUPPORT)_SRC += ecmd_parser/network.c
$(PORTIO_SUPPORT)_SRC += ecmd_parser/portio.c
$(PORTIO_SIMPLE_SUPPORT)_SRC += ecmd_parser/portio.c
$(I2C_MASTER_SUPPORT)_SRC += ecmd_parser/i2c.c
$(PWM_SUPPORT)_SRC += ecmd_parser/pwm.c
$(ALIASCMD_SUPPORT)_SRC += ecmd_parser/aliascmd.c
$(ECMD_SCRIPT_SUPPORT)_SRC += ecmd_parser/scripting.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################
# special stuff for this subdirectory

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
named_pin_simple_files=$(TOPDIR)/core/portio/np_simple.m4 ecmd_parser/np_simple_glue.m4
endif
endif

ecmd_parser/ecmd.c: ecmd_parser/ecmd_defs.c
ecmd_parser/ecmd_defs.c: ecmd_parser/ecmd_magic.m4 ecmd_parser/ecmd_defs.m4 $(named_pin_simple_files)
	@m4 $^ > $@.tmp
	@mv -f $@.tmp $@

ecmd_parser/np_simple_glue.m4: $(TOPDIR)/core/portio/config $(TOPDIR)/core/portio/np_simple_glue.awk
	@grep -v -e "^#" < $< | gawk -f $(TOPDIR)/core/portio/np_simple_glue.awk > $@

wiki.txt: wiki.m4 ecmd_defs.m4
	@m4 $^ | perl -ne 'unless(m/^\s*$$/){s/^\s+//;s/\s+$$//;print "$$_\n"}' > $@.tmp
	@mv -f $@.tmp $@

# extend normal clean rule
CLEAN_FILES += ecmd_parser/ecmd_defs.c ecmd_parser/np_simple_glue.m4 ecmd_parser/wiki.txt
