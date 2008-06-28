TARGET = libcontrol6.a
TOPDIR = ..
SRC = \
	control6.c

##############################################################################
# generic fluff

include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

control6.c: control6.m4 control6.src
	m4 $^ > $@

clean: clean-common
	rm -f 6control.c

