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
	hardware/i2c/master/i2c_pcf8574x.c \
	hardware/i2c/master/i2c_max7311.c \
	hardware/i2c/master/i2c_pca9685.c \

$(I2C_PCA9555_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_pca9555.c \
	hardware/i2c/master/i2c_pca9555_ecmd.c \

$(I2C_DS13X7_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_ds13x7.c

$(I2C_PCF8583_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_pcf8583.c

$(I2C_24CXX_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_24CXX.c

$(I2C_TSL2550_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_tsl2550.c

$(I2C_TSL2561_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_tsl2561.c

$(I2C_BMP085_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_bmp085.c

$(I2C_UDP_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_udp.c \
	hardware/i2c/master/i2c_udp_net.c

$(VFS_EEPROM_RAW_SUPPORT)_SRC += hardware/i2c/master/vfs_eeprom_raw.c
$(VFS_EEPROM_SUPPORT)_SRC += hardware/i2c/master/vfs_eeprom.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
