TOPDIR ?= ..
include $(TOPDIR)/.config

$(CONTROL6_SUPPORT)_SRC += control6/control6.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

CONTROL6_C_SRC = control6/control6.m4 control6/control6.src
CONTROL6_H_SRC = control6/control6-header.m4 control6/control6.src

control6/control6.h: named_pin/user_config.c ${CONTROL6_H_SRC}
	$(M4) $(M4FLAGS) ${CONTROL6_H_SRC} > $@

control6/control6.c: named_pin/user_config.c ${CONTROL6_C_SRC}
	$(M4) $(M4FLAGS) ${CONTROL6_C_SRC} > $@

CLEAN_FILES += control6/control6.c control6/control6.h

