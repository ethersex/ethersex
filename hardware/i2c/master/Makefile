TOPDIR ?= ../../..
include $(TOPDIR)/.config

$(I2C_MASTER_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_master.c \
	hardware/i2c/master/i2c_pca9531.c \
	hardware/i2c/master/i2c_lm75.c \
	hardware/i2c/master/i2c_pcf8574x.c \

$(I2C_24CXX_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_24CXX.c

$(I2C_UDP_SUPPORT)_SRC += \
	hardware/i2c/master/i2c_udp.c \
	hardware/i2c/master/i2c_udp_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
