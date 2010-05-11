TOPDIR ?= ../..
include $(TOPDIR)/.config

$(HC165_SUPPORT)_SRC += hardware/io_expander/hc165.c
$(HC595_SUPPORT)_SRC += hardware/io_expander/hc595.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

ifneq ($(rootbuild),t)
hc165.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps hc165.sch
	ps2pdf hc165.ps
	$(RM) -rf hc165.ps

hc595.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps hc595.sch
	ps2pdf hc595.ps
	$(RM) -rf hc595.ps
endif

CLEAN_FILES += hardware/io_expander/hc165.pdf hardware/io_expander/gschem.log
CLEAN_FILES += hardware/io_expander/hc165.pdf

