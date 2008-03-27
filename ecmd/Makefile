TARGET = libecmd.a
TOPDIR = ..
SRC = \
      	adc.c		\
      	clock.c		\
      	dns.c		\
	ecmd.c 		\
	fs20.c		\
	hd44780.c	\
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

ecmd.c: ecmd_defs.c
ecmd_defs.c: ecmd_defs.m4
	m4 $< > $@

# extend normal clean rule
clean: clean-common
	$(RM) ecmd_defs.c
