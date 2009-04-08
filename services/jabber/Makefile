TOPDIR ?= ..
include $(TOPDIR)/.config

$(JABBER_SUPPORT)_SRC += services/jabber/jabber.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

services/jabber/jabber.c: services/jabber/known_buddies.c
services/jabber/known_buddies.c: services/jabber/known_buddies.src services/jabber/known_buddies.m4
	sed 's/^/USER(/; s/$$/)/' services/jabber/known_buddies.src | $(M4) services/jabber/known_buddies.m4 - > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
CLEAN_FILES += services/jabber/known_buddies.c

