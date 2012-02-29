TOPDIR ?= ../..
include $(TOPDIR)/.config

$(CLOCK_SUPPORT)_SRC += services/clock/clock.c 
$(CLOCK_SUPPORT)_SRC += services/clock/clock_lib.c 
$(CLOCK_SUPPORT)_SOAP_SRC += services/clock/clock_soap.c

$(CLOCK_DATETIME_SUPPORT)_ECMD_SRC += services/clock/clock_ecmd_date.c
$(CLOCK_DATETIME_SUPPORT)_ECMD_SRC += services/clock/clock_ecmd_time.c

$(CLOCK_DATE_SUPPORT)_ECMD_SRC += services/clock/clock_ecmd_date.c
$(CLOCK_TIME_SUPPORT)_ECMD_SRC += services/clock/clock_ecmd_time.c

$(WHM_SUPPORT)_ECMD_SRC += services/clock/whm_ecmd.c
$(WHM_SUPPORT)_SOAP_SRC += services/clock/whm_soap.c

$(UPTIME_SUPPORT)_ECMD_SRC += services/clock/uptime_ecmd.c


##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
