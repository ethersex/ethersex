TOPDIR ?= ../..
include $(TOPDIR)/.config

$(ZBUS_SUPPORT)_SRC += protocols/zbus/process.c protocols/zbus/zbus.c
$(ZBUS_RAW_SUPPORT)_SRC += protocols/zbus/zbus_raw_net.c
$(ZBUS_ECMD)_ECMD_SRC += protocols/zbus/zbus_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
