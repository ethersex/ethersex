TOPDIR ?= ../..
include $(TOPDIR)/.config

$(CLOCK_SUPPORT)_SRC += services/clock/clock.c 
$(CLOCK_DATETIME_SUPPORT)_ECMD_SRC += services/clock/clock_ecmd.c

$(WHM_SUPPORT)_ECMD_SRC += services/clock/whm_ecmd.c
$(WHM_SUPPORT)_SOAP_SRC += services/clock/whm_soap.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
