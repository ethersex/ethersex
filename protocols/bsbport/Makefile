TOPDIR ?= ../..
include $(TOPDIR)/.config

# The order does matter, bsbport.c must be listed before bsbport_net.c because
# of meta call order!
$(BSBPORT_SUPPORT)_SRC += protocols/bsbport/bsbport.c protocols/bsbport/bsbport_net.c protocols/bsbport/bsbport_helper.c protocols/bsbport/bsbport_rx.c protocols/bsbport/bsbport_tx.c 
$(BSBPORT_POLLING_SUPPORT)_SRC += protocols/bsbport/bsbport_polling.c
$(BSBPORT_SUPPORT)_ECMD_SRC += protocols/bsbport/bsbport_ecmd.c
$(BSBPORT_MQTT_SUPPORT)_SRC += protocols/bsbport/bsbport_mqtt.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
