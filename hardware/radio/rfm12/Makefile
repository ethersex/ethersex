TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(RFM12_SUPPORT)_SRC += hardware/radio/rfm12/rfm12.c

$(RFM12_IP_SUPPORT)_SRC += hardware/radio/rfm12/process.c

$(RFM12_ASK_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_ask.c
$(RFM12_ASK_SENSING_SUPPORT)_SRC += hardware/radio/rfm12/ask_sense.c

$(RFM12_RAW_SUPPORT)_SRC += hardware/radio/rfm12/rfm12_raw_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

schematic.pdf: schematic.sch
	$(TOPDIR)/contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps

CLEAN_FILES += hardware/radio/rfm12/schematic.pdf
CLEAN_FILES += hardware/radio/rfm12/gschem.log
