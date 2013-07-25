TOPDIR ?= ../..
include $(TOPDIR)/.config

$(HD44780_SUPPORT)_SRC      += hardware/lcd/hd44780_core.c
$(HD44780_DIREKT)_SRC       += hardware/lcd/hd44780.c
$(HD44780_I2CSUPPORT)_SRC   += hardware/lcd/hd44780_i2c.c
$(HD44780_SERLCD)_SRC       += hardware/lcd/hd44780_serlcd.c
$(HD44780_2WIRE)_SRC        += hardware/lcd/hd44780_2wire.c
$(HD44780_SUPPORT)_ECMD_SRC += hardware/lcd/ecmd.c

$(HR20_LCD_SUPPORT_FULL)_SRC += hardware/lcd/hr20.c
$(HR20_LCD_SUPPORT_FULL)_ECMD_SRC += hardware/lcd/hr20-ecmd.c

$(HR20_LCD_SUPPORT)_SRC += hardware/lcd/hr20.c
$(HR20_LCD_ECMD_SUPPORT)_ECMD_SRC += hardware/lcd/hr20-ecmd.c


##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

ifneq ($(rootbuild),t)
hd44780.pdf: hd44780.sch
	$(TOPDIR)/contrib/gschem/sch2ps hd44780.sch
	ps2pdf hd44780.ps
	$(RM) -f hd44780.ps
endif

CLEAN_FILES += hardware/lcd/hd44780.pdf hardware/lcd/gschem.log

