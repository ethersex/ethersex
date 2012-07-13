TOPDIR ?= ../..
include $(TOPDIR)/.config

$(CRC_PAD_SUPPORT)_ECMD_SRC += core/crc/crc_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

-include crc16-concat.d
crc16-concat: crc16-concat.c
	@$(HOSTCC) -Wall -W -ggdb -O2 -o $@ $<

# extend normal clean rule
CLEAN_FILES += core/crc/crc16.c
