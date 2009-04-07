TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(SD_READER_SUPPORT)_SRC +=			\
	storage/sd_reader/byteordering.c	\
	storage/sd_reader/fat.c			\
	storage/sd_reader/main.c		\
	storage/sd_reader/partition.c		\
	storage/sd_reader/sd_raw.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

