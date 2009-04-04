TOPDIR ?= ..
include $(TOPDIR)/.config

$(MODBUS_SUPPORT)_SRC += modbus/modbus.c \
	modbus/modbus_client.c \
	modbus/modbus_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

modbus/modbus_client.c: modbus/modbus_client_cb.c
