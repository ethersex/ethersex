TOPDIR ?= ..
include $(TOPDIR)/.config

$(HD44780_SUPPORT)_SRC += lcd/hd44780.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += lcd/schematic.pdf lcd/gschem.log
