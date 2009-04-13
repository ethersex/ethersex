TOPDIR ?= ../..
include $(TOPDIR)/.config

$(HD44780_SUPPORT)_SRC += hardware/lcd/hd44780.c
$(HD44780_SUPPORT)_ECMD_SRC += hardware/lcd/ecmd.c

$(LCD_MENU_SUPPORT)_SRC += hardware/lcd/menu.c hardware/lcd/menu_defs.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += hardware/lcd/schematic.pdf hardware/lcd/gschem.log


MENU_DEFS += $(TOPDIR)/hardware/lcd/menu_defs.m4

$(TOPDIR)/hardware/lcd/menu_defs.c: $(TOPDIR)/hardware/lcd/menu_magic.m4 $(MENU_DEFS)
	$(M4) $^ > $@

CLEAN_FILES += $(TOPDIR)/hardware/lcd/menu_defs.c
