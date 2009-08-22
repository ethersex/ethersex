TOPDIR ?= ../..
include $(TOPDIR)/.config

$(VFS_SUPPORT)_HTTPD_FILES += services/httpd/handle_vfs.c
$(ECMD_PARSER_SUPPORT)_HTTPD_FILES += services/httpd/handle_ecmd.c
$(HTTP_SD_DIR_SUPPORT)_HTTPD_FILES += services/httpd/handle_sd_dir.c
$(HTTPD_SOAP_SUPPORT)_HTTPD_FILES += services/httpd/handle_soap.c

$(MIME_SUPPORT)_SRC += services/httpd/magic.c

$(HTTPD_SUPPORT)_SRC += services/httpd/handle_400.c	\
	services/httpd/handle_401.c	\
	services/httpd/handle_404.c	\
	services/httpd/headers.c		\
	services/httpd/httpd.c		\
	services/httpd/base64_mit.c	\
	$(y_HTTPD_FILES)

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

$(TOPDIR)/services/httpd/magic.c: $(TOPDIR)/services/httpd/compile-magic $(TOPDIR)/services/httpd/magic.mime
	$^ > $@

CLEAN_FILES += services/httpd/magic.c

