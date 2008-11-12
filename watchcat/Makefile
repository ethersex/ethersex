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
user_config.h: config watchcat.m4
	m4 watchcat.m4 config > user_config.h

# extend normal clean rule
clean: clean-common
	$(RM) user_config.h
