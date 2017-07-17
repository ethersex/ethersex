TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(BLUETOOTH_SUPPORT)_SRC += \
	hardware/radio/bluetooth/bt.c \
	hardware/radio/bluetooth/bt_usart.c

$(ARCH_AVR)_ECMD_SRC += hardware/radio/bluetooth/bt_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
