TOPDIR ?= ..
include $(TOPDIR)/.config

m4_lang = $(sort $(wildcard $(TOPDIR)/control6/lang.d/*.m4))

$(CONTROL6_SUPPORT)_SRC += control6/control6.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

C6_SOURCE ?= $(TOPDIR)/control6/control6.src

CONTROL6_C_SRC = ${m4_lang} ${C6_SOURCE}
CONTROL6_H_SRC = $(TOPDIR)/control6/control6-header.m4 $(C6_SOURCE)

$(TOPDIR)/control6/control6.h: $(TOPDIR)/core/portio/user_config.c ${CONTROL6_H_SRC}
	$(M4) $(M4FLAGS) ${CONTROL6_H_SRC} > $@

$(TOPDIR)/control6/control6.c: $(TOPDIR)/core/portio/user_config.c ${CONTROL6_C_SRC}
	$(M4) $(M4FLAGS) ${CONTROL6_C_SRC} > $@

CLEAN_FILES += control6/control6.c control6/control6.h

