TARGET = libwatchcat.a
TOPDIR = ..
SRC = \
	watchcat.c \

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################

watchcat.c: user_config.h
user_config.h: config
	$(@D)/cfgpp > $@

# extend normal clean rule
clean: clean-common
	$(RM) user_config.h
