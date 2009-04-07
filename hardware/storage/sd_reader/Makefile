TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(SD_READER_SUPPORT)_SRC +=				\
	hardware/storage/sd_reader/byteordering.c	\
	hardware/storage/sd_reader/fat.c		\
	hardware/storage/sd_reader/init.c		\
	hardware/storage/sd_reader/partition.c		\
	hardware/storage/sd_reader/sd_raw.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

