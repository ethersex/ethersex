TARGET = libecmd.a
TOPDIR = ..
SRC = \
	ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

ecmd.c: ecmd_defs.c
ecmd_defs.c: ecmd_defs.m4
	m4 $< > $@

# extend normal clean rule
clean: clean-common
	$(RM) ecmd_defs.c
