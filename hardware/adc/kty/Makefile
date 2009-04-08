TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(KTY_SUPPORT)_SRC += hardware/adc/kty/kty81.c
$(KTY_SUPPORT)_ECMD_SRC += hardware/adc/kty/ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps hc165.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += hardware/adc/kty/schematic.pdf hardware/adc/kty/gschem.log

