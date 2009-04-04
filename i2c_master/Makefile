TOPDIR ?= ..
include $(TOPDIR)/.config

$(I2C_MASTER_SUPPORT)_SRC += \
	i2c_master/i2c_master.c \
	i2c_master/i2c_24CXX.c \
	i2c_master/i2c_pca9531.c \
	i2c_master/i2c_lm75.c \
	i2c_master/i2c_pcf8574x.c \
	i2c_master/i2c_udp.c \
	i2c_master/i2c_udp_net.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk
