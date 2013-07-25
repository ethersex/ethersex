TOPDIR ?= ../..
include $(TOPDIR)/.config

$(CRON_SUPPORT)_SRC += services/cron/cron.c services/cron/cron_ecmd.c
$(CRON_SUPPORT_TEST)_SRC += services/cron/test.c
$(CRON_STATIC_SUPPORT)_SRC += services/cron/cron_static.c
$(CRON_SHARED_SUPPORT)_SRC += services/cron/cron_shared.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
