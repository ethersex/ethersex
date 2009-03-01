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

pwm.c: ethersex_wav.h
ethersex_wav.h: ethersex.wav
	../scripts/soundinclude ethersex.wav > ethersex_wav.h

# extend normal clean rule
clean: clean-common
	$(RM) ethersex_wav.h
