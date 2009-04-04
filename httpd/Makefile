TOPDIR ?= ..
include $(TOPDIR)/.config

$(VFS_SUPPORT)_HTTPD_FILES += httpd/handle_vfs.c
$(ECMD_PARSER_SUPPORT)_HTTPD_FILES += httpd/handle_ecmd.c

$(HTTPD_SUPPORT)_SRC += httpd/handle_400.c	\
		httpd/handle_401.c	\
		httpd/handle_404.c	\
		httpd/headers.c		\
		httpd/httpd.c		\
		httpd/base64_mit.c	\
		$(y_HTTPD_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

