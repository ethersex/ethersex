TOPDIR ?= ..
include $(TOPDIR)/.config

$(CAST5_SUPPORT)_SRC += crypto/cast5.c
$(MD5_SUPPORT)_SRC += crypto/md5.c

$(SHA1_SUPPORT)_ASRC += crypto/sha1-asm.S

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
