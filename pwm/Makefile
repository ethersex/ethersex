TARGET = libpwm.a
TOPDIR = ..
SRC = pwm.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

# extend normal clean rule
clean: clean-common

