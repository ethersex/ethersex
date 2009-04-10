TOPDIR ?= ../..
include $(TOPDIR)/.config

$(WATCHCAT_SUPPORT)_SRC += services/watchcat/watchcat.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################

services/watchcat/watchcat.c: services/watchcat/user_config.h
services/watchcat/user_config.h: services/watchcat/watchcat.m4 services/watchcat/config
	$(M4) -DMCU=$(MCU) $^ > $@

# extend normal clean rule
CLEAN_FILES += services/watchcat/user_config.h
