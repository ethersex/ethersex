TOPDIR ?= ..
include $(TOPDIR)/.config

$(VFS_SUPPORT)_SRC += vfs/vfs.c
$(VFS_DC3840_SUPPORT)_SRC += vfs/vfs_dc3840.c
$(VFS_DF_SUPPORT)_SRC += vfs/vfs_df.c
$(VFS_EEPROM_RAW_SUPPORT)_SRC += vfs/vfs_eeprom_raw.c
$(VFS_EEPROM_SUPPORT)_SRC += vfs/vfs_eeprom.c
$(VFS_INLINE_SUPPORT)_SRC += vfs/vfs_inline.c
$(VFS_SD_SUPPORT)_SRC += vfs/vfs_sd.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

-include vfs-concat.d
vfs-concat: vfs-concat.c vfs_inline.h
	$(HOSTCC) -Wall -W -ggdb -O2 -o $@ $^

# extend normal clean rule
CLEAN_FILES += vfs/vfs-concat vfs/embed/*.tmp

