TOPDIR ?= ../..
include $(TOPDIR)/.config

$(UIP_SUPPORT)_SRC += protocols/uip/uip.c
$(UIP_SUPPORT)_SRC += protocols/uip/uip_multi.c
$(UIP_SUPPORT)_SRC += protocols/uip/uip_router.c
$(UIP_SUPPORT)_SRC += protocols/uip/parse.c

$(IPSTATS_SUPPORT)_ECMD_SRC += protocols/uip/ipstats.c

ifneq ($(TEENSY_SUPPORT),y)
$(UIP_SUPPORT)_ECMD_SRC += protocols/uip/ecmd.c
endif

ifeq ($(ETHERNET_SUPPORT),y)
$(IPV4_SUPPORT)_SRC += protocols/uip/uip_arp.c
$(IPV6_SUPPORT)_SRC += protocols/uip/uip_neighbor.c protocols/uip/ipv6.c
$(ETHERNET_SUPPORT)_SRC += protocols/uip/check_cache.c
endif

$(OPENVPN_SUPPORT)_SRC += protocols/uip/uip_openvpn.c
$(RFM12_IP_SUPPORT)_SRC += protocols/uip/uip_rfm12.c
$(ZBUS_SUPPORT)_SRC += protocols/uip/uip_zbus.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

CFLAGS += -fno-strict-aliasing
