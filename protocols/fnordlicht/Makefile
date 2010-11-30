TOPDIR ?= ../..
include $(TOPDIR)/.config

$(FNORDLICHT_SUPPORT)_SRC += protocols/fnordlicht/fnordlicht.c protocols/fnordlicht/soft_uart.c

$(FNORDLICHT_SERVO_SUPPORT)_SRC += protocols/fnordlicht/fnordlicht_servo.c

# $(DMX_SUPPORT)_ECMD_SRC += \
# 	protocols/artnet/ecmd.c
##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
