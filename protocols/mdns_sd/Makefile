TOPDIR ?= ../..
include $(TOPDIR)/.config

$(MDNS_SD_SUPPORT)_SRC += protocols/mdns_sd/mdns_sd.c \
	protocols/mdns_sd/mdns_sd_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

protocols/mdns_sd/mdns_sd.c: protocols/mdns_sd/mdns_services.c
protocols/mdns_sd/mdns_services.c: protocols/mdns_sd/mdns_services.m4
	$(M4) $< > $@

# extend normal clean rule
CLEAN_FILES += protocols/mdns_sd/mdns_services.c
