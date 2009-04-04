TOPDIR ?= ..
include $(TOPDIR)/.config

$(HC165_SUPPORT)_SRC += hc165/hc165.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += hc165/schematic.pdf hc165/gschem.log
