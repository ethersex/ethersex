TARGET = libusb.a
TOPDIR = ..
CFLAGS += -Iusbdrv -I.
ASFLAGS += -x assembler-with-cpp
ASFLAGS += -Iusbdrv -I.

SRC = \
	usb.c \
	usbdrv/usbdrv.c \
	ecmd_usb.c

ASRC = \
	usbdrv/usbdrvasm.S

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk


