TARGET = libecmd_parser.a
TOPDIR = ..
SRC = \
      	adc.c		\
      	clock.c		\
      	dataflash.c	\
      	dns.c		\
	ecmd.c 		\
	fs20.c		\
	hd44780.c	\
	httpd.c	\
	modbus.c	\
	network.c	\
	onewire.c	\
	portio.c	\
	rc5.c		\
	rfm12.c		\
	usart.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory


# We directly include the menuconfig config file, it seems to be suitable
# for inclusion.  It's just defining quite a few variables, the rest
# is commented out using hash signs ...
ifneq ($(no_deps),t)
include $(TOPDIR)/.config
endif

# If (full-featured) portio is disabled, but named_pin enabled, we
# need to do the named_pin simple awk/m4 hackery ...
ifneq ($(PORTIO_SUPPORT),y)
ifeq ($(NAMED_PIN_SUPPORT),y)
named_pin_simple_files=../named_pin/np_simple.m4 np_simple_glue.m4
endif
endif

ecmd.c: ecmd_defs.c
ecmd_defs.c: ecmd_magic.m4 ecmd_defs.m4 $(named_pin_simple_files)
	m4 $^ > $@.tmp
	mv -f $@.tmp $@

np_simple_glue.m4: ../named_pin/config ../named_pin/np_simple_glue.awk
	gawk -f ../named_pin/np_simple_glue.awk < $< > $@

wiki.txt: wiki.m4 ecmd_defs.m4
	m4 $^ | perl -ne 'unless(m/^\s*$$/){s/^\s+//;s/\s+$$//;print "$$_\n"}' > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
clean: clean-common
	$(RM) -f ecmd_defs.c np_simple_glue.m4 wiki.txt
