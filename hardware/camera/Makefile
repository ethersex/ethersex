TOPDIR ?= ../..
include $(TOPDIR)/.config

$(DC3840_SUPPORT)_SRC += hardware/camera/dc3840.c
$(DC3840_SUPPORT)_SRC += hardware/camera/dc3840-util.c

$(MCA25_SUPPORT)_SRC += hardware/camera/mca25.c

$(VFS_DC3840_SUPPORT)_SRC += hardware/camera/vfs_dc3840.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

CFLAGS += -fshort-enums
