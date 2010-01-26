TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(I2C_MASTER_SUPPORT)_ECMD_SRC += \
	hardware/i2c/master/ecmd.c

$(I2C_MASTER_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_master.c \
	hardware/i2c/master/i2c_generic.c \
	hardware/i2c/master/i2c_pca9531.c \
	hardware/i2c/master/i2c_lm75.c \
	hardware/i2c/master/i2c_ds1631.c \
	hardware/i2c/master/i2c_tsl2550.c \
	hardware/i2c/master/i2c_pcf8574x.c \
	hardware/i2c/master/i2c_max7311.c \

$(I2C_24CXX_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_24CXX.c

$(I2C_UDP_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_udp.c \
	hardware/i2c/master/i2c_udp_net.c

$(VFS_EEPROM_RAW_SUPPORT)_SRC += hardware/i2c/master/vfs_eeprom_raw.c
$(VFS_EEPROM_SUPPORT)_SRC += hardware/i2c/master/vfs_eeprom.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
