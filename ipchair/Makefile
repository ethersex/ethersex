TARGET = libipchair.a
TOPDIR = ..
SRC = \
	ipchair.c


##############################################################################
# generic fluff

include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

ipchair.c: ipchair.m4 userscript.m4
	m4 $^ > $@

clean: clean-common
	rm -f ipchair.c

