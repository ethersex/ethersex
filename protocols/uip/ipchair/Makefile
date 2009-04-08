TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(IPCHAIR_SUPPORT)_SRC += protocols/uip/ipchair/ipchair.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

IPCM4SRCS := protocols/uip/ipchair/ipchair.m4 protocols/uip/ipchair/rewrite.m4

ifeq ($(IPCHAIR_MASQ),y)
IPCM4SRCS += protocols/uip/ipchair/masquerade.m4
endif

IPCM4SRCS += protocols/uip/ipchair/userscript.m4

protocols/uip/ipchair/ipchair.c: $(IPCM4SRCS)
	$(M4) $^ > $@

CLEAN_FILES += protocols/uip/ipchair/ipchair.c

