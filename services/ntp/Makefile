TOPDIR ?= ../..
include $(TOPDIR)/.config

$(NTP_SUPPORT)_SRC += services/ntp/ntp.c services/ntp/ntp_net.c
$(NTPD_SUPPORT)_SRC += services/ntp/ntpd_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
