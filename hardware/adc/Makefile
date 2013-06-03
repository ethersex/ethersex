TOPDIR ?= ../..
include $(TOPDIR)/.config

$(ADC_LIGHT)_SRC += hardware/adc/adc.c

$(ADC_SUPPORT)_SRC += hardware/adc/adc.c
$(ADC_SUPPORT)_ECMD_SRC += hardware/adc/adc_ecmd.c

$(HR20_TEMP_SUPPORT)_SRC += hardware/adc/hr20-temp.c
$(HR20_TEMP_SUPPORT)_SRC += hardware/adc/hr20-batt.c

$(HR20_TEMP_ECMD_SUPPORT)_ECMD_SRC += hardware/adc/hr20-temp-ecmd.c

$(NEED_TEMP2TEXT)_SRC += hardware/adc/temp2text.c


##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
