TOPDIR ?= ../..
include $(TOPDIR)/.config

$(NAMED_PIN_SUPPORT)_NP_SRC += core/portio/ecmd_np.c

$(PORTIO_SUPPORT)_SRC += core/portio/portio.c ${y_NP_SRC}
$(PORTIO_SUPPORT)_ECMD_SRC += core/portio/ecmd.c

$(PORTIO_SIMPLE_SUPPORT)_ECMD_SRC += core/portio/ecmd_simple.c

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
NP_CONFIG ?= $(TOPDIR)/pinning/named_pin/default

core/portio/named_pin.c: core/portio/user_config.c
core/portio/user_config.c: $(NP_CONFIG)
	$(CONFIG_SHELL) core/portio/cfgpp $<

# extend normal clean rule
CLEAN_FILES += core/portio/user_config.h core/portio/user_config.c \
	core/portio/np_simple.c

core/portio/np_simple.c: $(NP_CONFIG) core/portio/np_simple.awk
	grep -v -e "^#" < $< | ${AWK} -f core/portio/np_simple.awk > $@.tmp
	mv -f $@.tmp $@

show-np-config-file:
	@echo $(NP_CONFIG)
export NP_CONFIG
