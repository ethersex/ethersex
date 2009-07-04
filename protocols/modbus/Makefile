TOPDIR ?= ../..
include $(TOPDIR)/.config

$(MODBUS_SUPPORT)_SRC += protocols/modbus/modbus.c \
	protocols/modbus/modbus_client.c \
	protocols/modbus/modbus_net.c

$(MODBUS_SUPPORT)_ECMD_SRC += protocols/modbus/modbus_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

protocols/modbus/modbus_client.c: protocols/modbus/modbus_client_cb.c
