TOPDIR ?= ..
include $(TOPDIR)/.config

$(I2C_SLAVE_SUPPORT)_SRC += hardware/i2c/slave/i2c_slave.c hardware/i2c/slave/i2c_slave_ecmd.c
#$(ECMD_I2C_SLAVE_SUPPORT)_SRC += hardware/i2c/slave/i2c_slave_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
