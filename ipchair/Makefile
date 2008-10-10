TARGET = libipchair.a
TOPDIR = ..
SRC = \
	ipchair.c


##############################################################################
# generic fluff

include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

M4SRCS := ipchair.m4 rewrite.m4

ifeq ($(IPCHAIR_MASQ),y)
M4SRCS += masquerade.m4
endif

M4SRCS += userscript.m4

ipchair.c: $(M4SRCS)
	m4 $^ > $@

clean: clean-common
	rm -f ipchair.c

