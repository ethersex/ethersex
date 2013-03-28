TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(RFM12_SUPPORT)_SRC += hardware/radio/rfm12/rfm12.c

ifneq (${TEENSY_SUPPORT},y)
$(RFM12_SUPPORT)_ECMD_SRC += hardware/radio/rfm12/rfm12_ecmd.c
endif

$(RFM12_IP_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_net.c

$(RFM12_ASK_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_ask.c
$(RFM12_ASK_SUPPORT)_ECMD_SRC += hardware/radio/rfm12/rfm12_ask_ecmd.c
$(RFM12_ASK_SENSING_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_ask_sense.c

$(RFM12_RAW_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_raw_net.c

$(RFM12_ASK_FS20_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_fs20.c
$(RFM12_ASK_FS20_SUPPORT)_ECMD_SRC += hardware/radio/rfm12/rfm12_fs20_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

schematic.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps

CLEAN_FILES += hardware/radio/rfm12/schematic.pdf
CLEAN_FILES += hardware/radio/rfm12/gschem.log
