TOPDIR ?= ../..
include $(TOPDIR)/.config

$(SCHEDULER_SUPPORT)_SRC += core/scheduler/scheduler.c
$(SCHEDULER_DYNAMIC_SUPPORT)_SRC += core/scheduler/dynamic.c

$(DEBUG_SCHEDULER_SUPPORT)_SRC += core/scheduler/scheduler_test.c
##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
