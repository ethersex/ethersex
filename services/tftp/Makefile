TOPDIR ?= ../..
include $(TOPDIR)/.config

$(BOOTLOADER_SUPPORT)_TFTP_FILES += services/tftp/tftp-bootload.c
$(VFS_SUPPORT)_TFTP_FILES += services/tftp/tftp-vfs.c

$(TFTP_SUPPORT)_SRC += services/tftp/tftp_net.c $(y_TFTP_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
