TOPDIR ?= ../..
include $(TOPDIR)/.config

$(GUI_SUPPORT)_SRC += core/gui/font.c 
$(GUI_SUPPORT)_SRC += core/gui/matek.c 
$(GUI_SUPPORT)_SRC += core/gui/geometric.c 

MATEK_SOURCE=$(TOPDIR)/core/gui/matek/test.m4

$(TOPDIR)/core/gui/matek.c: $(TOPDIR)/core/gui/matek.m4 $(MATEK_SOURCE)
	m4 $^ -DARCH_AVR=${ARCH_AVR} > $@


CLEAN_FILES += core/gui/matek.c
##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
