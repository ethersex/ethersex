TOPDIR ?= ..
include $(TOPDIR)/.config

$(CRON_SUPPORT)_SRC += cron/cron.c
$(CRON_SUPPORT_TEST)_SRC += cron/test.c

$(CRON_STATIC_SUPPORT)_SRC += cron/cron_static.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
