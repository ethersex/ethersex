TOPDIR ?= ../..
include $(TOPDIR)/.config

$(HD44780_SUPPORT)_SRC += hardware/lcd/hd44780.c
$(HD44780_SUPPORT)_ECMD_SRC += hardware/lcd/ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += hardware/lcd/schematic.pdf hardware/lcd/gschem.log
