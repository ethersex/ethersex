TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(DATAFLASH_SUPPORT)_SRC += hardware/storage/dataflash/df.c hardware/storage/dataflash/fs.c
$(DATAFLASH_SUPPORT)_ECMD_SRC += hardware/storage/dataflash/ecmd.c
$(VFS_DF_SUPPORT)_SRC += hardware/storage/dataflash/vfs_df.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
