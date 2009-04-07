TOPDIR ?= ../..
include $(TOPDIR)/.config

$(VFS_SUPPORT)_HTTPD_FILES += services/httpd/handle_vfs.c
$(ECMD_PARSER_SUPPORT)_HTTPD_FILES += services/httpd/handle_ecmd.c

$(HTTPD_SUPPORT)_SRC += services/httpd/httpd_ecmd.c services/httpd/handle_400.c	\
	services/httpd/handle_401.c	\
	services/httpd/handle_404.c	\
	services/httpd/headers.c		\
	services/httpd/httpd.c		\
	services/httpd/base64_mit.c	\
	$(y_HTTPD_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

