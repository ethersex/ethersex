TOPDIR ?= ..
include $(TOPDIR)/.config

$(HC595_SUPPORT)_SRC += hc595/hc595.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += hc595/schematic.pdf hc595/gschem.log
