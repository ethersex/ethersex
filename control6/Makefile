TARGET = libcontrol6.a
TOPDIR = ..
SRC = \
	control6.c

#M4FLAGS=-DLANGUAGE=GERMAN

##############################################################################
# generic fluff

include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

control6.c: control6.m4 control6.src
	m4 $(M4FLAGS) $^ > $@

clean: clean-common
	rm -f 6control.c

