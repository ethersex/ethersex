TOPDIR ?= ../..
include $(TOPDIR)/.config

$(DHT_SUPPORT)_SRC += hardware/dht/dht.c
$(DHT_SUPPORT)_ECMD_SRC += hardware/dht/dht_ecmd.c

hardware/dht/dht.c: hardware/dht/dht_config.h
hardware/dht/dht_config.h: hardware/dht/dht_pinning.conf
	$(CONFIG_SHELL) hardware/dht/cfgdht <$< >$@

CLEAN_FILES += hardware/dht/dht_config.h

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
