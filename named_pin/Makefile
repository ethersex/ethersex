TARGET = libnamed_pin.a
TOPDIR = ..

# We directly include the menuconfig config file, it seems to be suitable
# for inclusion.  It's just defining quite a few variables, the rest
# is commented out using hash signs ...
include $(TOPDIR)/.config

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
np_simple_files = np_simple.c
endif
endif

SRC = \
	named_pin.c \
	$(np_simple_files)

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################
named_pin.c: user_config.h
user_config.h: config
	$(@D)/cfgpp > $@

# extend normal clean rule
clean: clean-common
	$(RM) -f user_config.h np_simple.c

np_simple.c: config np_simple.awk
	mawk -f np_simple.awk < $< > $@.tmp
	mv -f $@.tmp $@

