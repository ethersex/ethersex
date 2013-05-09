TOPDIR ?= ../..
include $(TOPDIR)/.config

$(ONEWIRE_SUPPORT)_SRC += hardware/onewire/onewire.c
$(ONEWIRE_SUPPORT)_ECMD_SRC += hardware/onewire/onewire_ecmd.c

$(ONEWIRE_DS2450_SUPPORT)_SRC += hardware/onewire/ds2450.c
$(ONEWIRE_DS2450_SUPPORT)_ECMD_SRC += hardware/onewire/ds2450_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
