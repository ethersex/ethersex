TARGET = liblcd.a
TOPDIR = ..
SRC = \
	hd44780.c \

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
