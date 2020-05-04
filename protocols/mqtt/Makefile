TOPDIR ?= ../..
include $(TOPDIR)/.config

$(MQTT_SUPPORT)_SRC += protocols/mqtt/mqtt.c
$(MQTT_SUPPORT)_SRC += protocols/mqtt/static_configuration.c

##############################################################################
# static configuration

$(ARCH_AVR)_AUTOGEN_SRC += protocols/mqtt/mqtt_callback_configuration.c

protocols/mqtt/mqtt_callback_configuration.c: meta.defines protocols/mqtt/meta_mqtt_callback_configuration.m4 meta.m4
	$(M4) `cat meta.defines` $(filter-out $<,$^) > $@

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

