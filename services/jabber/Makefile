TOPDIR ?= ..
include $(TOPDIR)/.config

$(JABBER_SUPPORT)_SRC += services/jabber/jabber.c
$(JABBER_EEPROM_SUPPORT)_ECMD_SRC += services/jabber/jabber_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

services/jabber/jabber.c: services/jabber/known_buddies.c
services/jabber/known_buddies.c: services/jabber/known_buddies.src services/jabber/known_buddies.m4
	$(SED) 's/^/USER(/; s/$$/)/' services/jabber/known_buddies.src | $(M4) services/jabber/known_buddies.m4 - > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
CLEAN_FILES += services/jabber/known_buddies.c

