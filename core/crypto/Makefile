TOPDIR ?= ../..
include $(TOPDIR)/.config

$(CAST5_SUPPORT)_SRC += core/crypto/cast5.c
$(MD5_SUPPORT)_ASRC += core/crypto/md5-asm.S
$(SHA1_SUPPORT)_ASRC += core/crypto/sha1-asm.S

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
