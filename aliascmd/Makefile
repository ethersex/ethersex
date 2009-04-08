TARGET = libaliascmd.a
TOPDIR = ..
SRC = aliascmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/scripts/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

aliascmd.c: alias_defs.c
alias_defs.c: alias_magic.m4 alias_defs.m4
	m4 $^ > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
clean: clean-common
	$(RM) -f alias_defs.c

