TOPDIR ?= ../..
include $(TOPDIR)/.config

$(ALIASCMD_SUPPORT)_SRC += protocols/ecmd/aliascmd.c

protocols/ecmd/aliascmd.c: protocols/ecmd/alias_defs.c
protocols/ecmd/alias_defs.c: protocols/ecmd/alias_magic.m4 protocols/ecmd/alias_defs.m4
	$(M4) $^ > $@.tmp
	mv -f $@.tmp $@

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

# extend normal clean rule
CLEAN_FILES += protocols/ecmd/alias_defs.c
