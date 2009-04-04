TOPDIR ?= ..
include $(TOPDIR)/.config

$(IPCHAIR_SUPPORT)_SRC += ipchair/ipchair.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

IPCM4SRCS := ipchair/ipchair.m4 ipchair/rewrite.m4

ifeq ($(IPCHAIR_MASQ),y)
IPCM4SRCS += ipchair/masquerade.m4
endif

IPCM4SRCS += ipchair/userscript.m4

ipchair.c: $(IPCM4SRCS)
	$(M4) $^ > $@

CLEAN_FILES += ipchair/ipchair.c

