TARGET = libusb.a
TOPDIR = ..
CFLAGS += -Iusbdrv -I.
ASFLAGS += -x assembler-with-cpp
ASFLAGS += -Iusbdrv -I.

SRC = \
	usb.c \
	usbdrv/usbdrv.c \
	ecmd_usb.c \
	usb_net.c

ASRC = \
	usbdrv/usbdrvasm.S

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

schematic.pdf: schematic.sch
	sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps

clean: clean-common
	$(RM) -rf schematic.pdf gschem.log usbdrv/usbdrv.d usbdrv/usbdrvasm.d



