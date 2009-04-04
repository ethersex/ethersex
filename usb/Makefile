TOPDIR ?= ..
include $(TOPDIR)/.config

CFLAGS += -Iusb/usbdrv -Iusb
ASFLAGS += -x assembler-with-cpp
ASFLAGS += -Iusbdrv -I.

$(USB_SUPPORT)_SRC += usb/usb.c usb/usbdrv/usbdrv.c
$(USB_NET_SUPPORT)_SRC += usb/usb_net.c
$(ECMD_USB_SUPPORT)_SRC += usb/ecmd_usb.c

$(USB_SUPPORT)_ASRC += usb/usbdrv/usbdrvasm.S

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += usb/schematic.pdf usb/gschem.log \
	usb/usbdrv/usbdrv.d usb/usbdrv/usbdrvasm.d



