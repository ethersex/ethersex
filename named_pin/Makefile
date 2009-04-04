TOPDIR ?= ..
include $(TOPDIR)/.config

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
np_simple_files = named_pin/np_simple.c
endif
endif

$(NAMED_PIN_SUPPORT)_SRC += named_pin/named_pin.c \
	$(np_simple_files)

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

##############################################################################
named_pin/named_pin.c: named_pin/user_config.c
named_pin/user_config.c: named_pin/config
	$(@D)/cfgpp > $@

# extend normal clean rule
CLEAN_FILES += named_pin/user_config.h named_pin/user_config.c \
	named_pin/np_simple.c

named_pin/np_simple.c: named_pin/config named_pin/np_simple.awk
	grep -v -e "^#" < $< | ${AWK} -f named_pin/np_simple.awk > $@.tmp
	mv -f $@.tmp $@

