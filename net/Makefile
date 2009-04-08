TOPDIR ?= ..
include $(TOPDIR)/.config

$(ECMD_SUPPORT)_SRC += net/ecmd_net.c
$(UECMD_SUPPORT)_SRC += net/uecmd_net.c
$(ECMD_SENDER_SUPPORT)_SRC += net/ecmd_sender_net.c
$(UECMD_SENDER_SUPPORT)_SRC += net/uecmd_sender_net.c
$(SENDMAIL_SUPPORT)_SRC += net/sendmail.c
$(UDP_ECHO_NET_SUPPORT)_SRC += net/udp_echo_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
