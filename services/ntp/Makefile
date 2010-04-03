TOPDIR ?= ../..
include $(TOPDIR)/.config

$(NTP_SUPPORT)_SRC += services/ntp/ntp.c
$(NTP_SUPPORT)_ECMD_SRC += services/ntp/ntp_ecmd.c
$(NTPD_SUPPORT)_SRC += services/ntp/ntpd_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
