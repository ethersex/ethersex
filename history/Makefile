TOPDIR ?= ..
include $(TOPDIR)/.config

$(I2C_DS13X7_SUPPORT)_ECMD_SRC += \
	history/ecmd.c

$(I2C_PCF8583_SUPPORT)_ECMD_SRC += \
	history/ecmd.c

$(I2C_24CXX_SUPPORT)_ECMD_SRC += \
	history/history.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################
# special stuff for this subdirectory

