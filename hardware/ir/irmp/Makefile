TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(IRMP_SUPPORT)_SRC += hardware/ir/irmp/irmp.c
$(IRMP_ECMD)_ECMD_SRC += hardware/ir/irmp/irmp_ecmd.c
$(REMOTE_IRMP_SUPPORT)_SRC += hardware/ir/irmp/remote_irmp.c
ifeq ($(MQTT_SUPPORT),y)
$(IRMP_SUPPORT)_SRC += hardware/ir/irmp/irmp_mqtt.c
endif

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
