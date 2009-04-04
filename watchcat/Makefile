TOPDIR ?= ..
include $(TOPDIR)/.config

$(WATCHCAT_SUPPORT)_SRC += watchcat/watchcat.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

##############################################################################

watchcat/watchcat.c: watchcat/user_config.h
watchcat/user_config.h: watchcat/watchcat.m4 watchcat/config
	$(M4) -DMCU=$(MCU) $^ > $@

# extend normal clean rule
CLEAN_FILES += watchcat/user_config.h
