TOPDIR ?= ../..
include $(TOPDIR)/.config

$(VFS_SUPPORT)_SRC += core/vfs/vfs.c
$(VFS_SUPPORT)_SRC += core/vfs/vfs-util.c

$(VFS_INLINE_SUPPORT)_SRC += core/vfs/vfs_inline.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

-include vfs-concat.d
vfs-concat: vfs-concat.c vfs_inline.h
	@$(HOSTCC) -Wall -W -ggdb -O2 -o $@ $<

# extend normal clean rule
CLEAN_FILES += core/vfs/vfs-concat embed/*.tmp

