TOPDIR ?= ..
include $(TOPDIR)/.config

$(JABBER_SUPPORT)_SRC += jabber/jabber.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

jabber/jabber.c: jabber/known_buddies.c
jabber/known_buddies.c: jabber/known_buddies.src jabber/known_buddies.m4
	sed 's/^/USER(/; s/$$/)/' jabber/known_buddies.src | $(M4) jabber/known_buddies.m4 - > $@.tmp
	mv -f $@.tmp $@
	
# extend normal clean rule
CLEAN_FILES += jabber/known_buddies.c

