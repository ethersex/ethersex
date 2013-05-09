TOPDIR ?= ../..
include $(TOPDIR)/.config

$(STELLA_SUPPORT)_SRC += \
	services/stella/stella_pwm.c \
	services/stella/stella.c
$(STELLA_SUPPORT)_ECMD_SRC += services/stella/stella_ecmd.c

#########################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
