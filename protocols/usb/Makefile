TOPDIR ?= ../..
include $(TOPDIR)/.config

CFLAGS += -Iprotocols/usb/usbdrv -Iprotocols/usb
ASFLAGS += -x assembler-with-cpp
ASFLAGS += -Iprotocols/usb/usbdrv -Iprotocols/usb

$(USB_SUPPORT)_SRC += protocols/usb/usb.c protocols/usb/usbdrv/usbdrv.c
$(USB_NET_SUPPORT)_SRC += protocols/usb/usb_net.c
$(USB_KEYBOARD_SUPPORT)_SRC += protocols/usb/usb_hid_keyboard.c
$(USB_MOUSE_SUPPORT)_SRC += protocols/usb/usb_hid_mouse.c
$(ECMD_USB_SUPPORT)_SRC += protocols/usb/ecmd_usb.c

$(USB_SUPPORT)_ASRC += protocols/usb/usbdrv/usbdrvasm.S

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

ifneq ($(rootbuild),t)
schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps
endif

CLEAN_FILES += protocols/usb/schematic.pdf protocols/usb/gschem.log \
	protocols/usb/usbdrv/usbdrv.d protocols/usb/usbdrv/usbdrvasm.d

protocols/usb/usbdrv/usbdrvasm.S: pinning.c

