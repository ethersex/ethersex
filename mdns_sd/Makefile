TARGET = libmdns_sd.a
TOPDIR = ..
SRC = \
	mdns_sd.c \
	mdns_sd_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

mdns_sd.c: mdns_services.c
mdns_services.c: mdns_services.m4
	m4 $< > $@

# extend normal clean rule
clean: clean-common
	$(RM) mdns_services.c
