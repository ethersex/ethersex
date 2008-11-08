TARGET = libhttpd.a
TOPDIR = ..
SRC = \
	httpd.c	\
	base64_mit.c	\

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

httpd-concat: httpd-concat.c httpd.h
	$(HOSTCC) -Wall -W -ggdb -O2 -o $@ $^

# extend normal clean rule
clean: clean-common
	$(RM) -f httpd-concat
