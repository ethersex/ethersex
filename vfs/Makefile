TARGET = libvfs.a
TOPDIR = ..
SRC = \
	vfs.c  			\
	vfs_dc3840.c		\
	vfs_df.c		\
	vfs_inline.c		\
	vfs_eeprom.c	\
	vfs_eeprom_raw.c	\
	vfs_sd.c


##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

-include vfs-concat.d
vfs-concat: vfs-concat.c
	$(HOSTCC) -Wall -W -ggdb -O2 -o $@ $^

# extend normal clean rule
clean: clean-common
	$(RM) -f vfs-concat  embed/*.tmp

