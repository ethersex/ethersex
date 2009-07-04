TOPDIR ?= ../..
include $(TOPDIR)/.config

ifneq ($(TEENSY_SUPPORT),y)
$(ENC28J60_SUPPORT)_ECMD_SRC += hardware/ethernet/enc28j60_ecmd.c
endif

$(ENC28J60_SUPPORT)_SRC += \
	hardware/ethernet/enc28j60.c		\
	hardware/ethernet/enc28j60_process.c	\
	hardware/ethernet/enc28j60_transmit.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
