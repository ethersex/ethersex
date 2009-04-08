TOPDIR ?= ../..
include $(TOPDIR)/.config

$(PORTIO_SUPPORT)_SRC += core/portio/portio.c

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
np_simple_files = core/portio/np_simple.c
endif
endif

$(NAMED_PIN_SUPPORT)_SRC += core/portio/named_pin.c \
	$(np_simple_files)

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################
core/portio/named_pin.c: core/portio/user_config.c
core/portio/user_config.c: core/portio/config
	$(@D)/cfgpp > $@

# extend normal clean rule
CLEAN_FILES += core/portio/user_config.h core/portio/user_config.c \
	core/portio/np_simple.c

core/portio/np_simple.c: core/portio/config core/portio/np_simple.awk
	grep -v -e "^#" < $< | ${AWK} -f core/portio/np_simple.awk > $@.tmp
	mv -f $@.tmp $@

