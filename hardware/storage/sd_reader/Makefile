TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(SD_READER_SUPPORT)_SRC +=				\
	hardware/storage/sd_reader/byteordering.c	\
	hardware/storage/sd_reader/fat.c		\
	hardware/storage/sd_reader/init.c		\
	hardware/storage/sd_reader/partition.c		\
	hardware/storage/sd_reader/sd_raw.c

$(SD_DATETIME_SUPPORT)_SRC += hardware/storage/sd_reader/datetime.c

$(VFS_SD_SUPPORT)_SRC += hardware/storage/sd_reader/vfs_sd.c
$(VFS_SD_SUPPORT)_ECMD_SRC += hardware/storage/sd_reader/ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

