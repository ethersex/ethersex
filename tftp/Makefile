TOPDIR ?= ..
include $(TOPDIR)/.config

$(BOOTLOADER_SUPPORT)_TFTP_FILES += tftp/tftp-bootload.c
$(VFS_SUPPORT)_TFTP_FILES += tftp/tftp-vfs.c

$(TFTP_SUPPORT)_SRC += tftp/tftp_net.c $(y_TFTP_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
