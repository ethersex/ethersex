TOPDIR ?= ..
include $(TOPDIR)/.config

$(MDNS_SD_SUPPORT)_SRC += mdns_sd/mdns_sd.c \
	mdns_sd/mdns_sd_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

mdns_sd/mdns_sd.c: mdns_sd/mdns_services.c
mdns_sd/mdns_services.c: mdns_sd/mdns_services.m4
	$(M4) $< > $@

# extend normal clean rule
CLEAN_FILES += mdns_sd/mdns_services.c
