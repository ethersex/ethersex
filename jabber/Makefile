TARGET = libjabber.a
TOPDIR = ..
SRC = \
	jabber.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

jabber.c: known_buddies.c
known_buddies.c: known_buddies.src known_buddies.m4
	sed 's/^/USER(/; s/$$/)/' known_buddies.src | m4 known_buddies.m4 - > $@.tmp
	mv -f $@.tmp $@
	
# extend normal clean rule
clean: clean-common
	$(RM) -f known_buddies.c

