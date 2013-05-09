TOPDIR ?= ../..
include $(TOPDIR)/.config

$(TLC5620_SUPPORT)_SRC += hardware/dac/tlc5620.c
$(TLC5620_SUPPORT)_ECMD_SRC += hardware/dac/ecmd.c

$(LTC1257_SUPPORT)_SRC += hardware/dac/ltc1257.c
$(LTC1257_SUPPORT)_ECMD_SRC += hardware/dac/ltc1257_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
