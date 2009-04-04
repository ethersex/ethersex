TOPDIR ?= ..
include $(TOPDIR)/.config

$(BOOTLOADER_SUPPORT)_TFTP_FILES += tftp/tftp-bootload.c
$(VFS_SUPPORT)_TFTP_FILES += tftp/tftp_net.c

$(TFTP_SUPPORT)_SRC += tftp/tftp-vfs.c $(y_TFTP_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
