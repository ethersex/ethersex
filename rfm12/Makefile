TARGET = librfm12.a
TOPDIR = ..
SRC = \
	process.c \
	ask_sense.c \
	rfm12.c \
	rfm12_ask.c \
	rfm12_raw_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

schematic.pdf: schematic.sch
	../contrib/gschem/sch2ps schematic.sch
	ps2pdf schematic.ps
	$(RM) -rf schematic.ps

clean: clean-common
	$(RM) -rf schematic.pdf gschem.log
