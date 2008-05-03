TARGET = libnamed_pin.a
TOPDIR = ..
SRC = \
	named_pin.c \

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
	$(RM) user_config.h
