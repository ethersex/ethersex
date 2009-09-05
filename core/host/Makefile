TOPDIR ?= ../..

$(TAP_SUPPORT)_SRC += core/host/tap.c
$(ARCH_HOST)_SRC += core/host/eeprom.c
$(ARCH_HOST)_ECMD_SRC += core/host/stdin.c
$(VFS_HOST_SUPPORT)_SRC += core/host/vfs.c

ifeq ($(ARCH_HOST),y)
LDFLAGS += $(shell pkg-config --libs glib-2.0)
CFLAGS += $(shell pkg-config --cflags glib-2.0)
endif

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

