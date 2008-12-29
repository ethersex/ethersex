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
	$(MAKE) -C ../named_pin user_config.c
	m4 $(M4FLAGS) $^ > $@

clean: clean-common
	rm -f control6.c

