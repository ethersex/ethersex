TOPDIR ?= ..
include $(TOPDIR)/.config

$(UIP_SUPPORT)_SRC += uip/uip_arp.c
$(UIP_SUPPORT)_SRC += uip/uip.c
$(UIP_SUPPORT)_SRC += uip/uip_multi.c
$(UIP_SUPPORT)_SRC += uip/uip_router.c

$(IPV6_SUPPORT)_SRC += uip/uip_neighbor.c
$(OPENVPN_SUPPORT)_SRC += uip/uip_openvpn.c
$(RFM12_IP_SUPPORT)_SRC += uip/uip_rfm12.c
$(ZBUS_SUPPORT)_SRC += uip/uip_zbus.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
