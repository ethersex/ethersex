TOPDIR ?= ../..
include $(TOPDIR)/.config

$(SYSLOG_SUPPORT)_SRC += protocols/syslog/syslog_net.c protocols/syslog/syslog.c
$(DEBUG_USE_SYSLOG)_SRC += protocols/syslog/syslog_debug.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
