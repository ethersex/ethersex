TOPDIR ?= ../..
include $(TOPDIR)/.config

$(FREQCOUNT_SUPPORT)_SRC += services/freqcount/freqcount_timer.c
$(FREQCOUNT_SUPPORT)_SRC += services/freqcount/freqcount_measure.c
$(FREQCOUNT_SUPPORT)_SRC += services/freqcount/freqcount_average.c
$(FREQCOUNT_SUPPORT)_ECMD_SRC += services/freqcount/freqcount_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
