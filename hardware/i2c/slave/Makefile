TOPDIR ?= ..
include $(TOPDIR)/.config

$(ECMD_I2C_SLAVE_SUPPORT)_SRC += hardware/i2c/slave/i2c_slave_ecmd.c
$(I2C_SLAVE_SUPPORT)_SRC += hardware/i2c/slave/i2c_slave.c $(ECMD_I2C_SLAVE_SUPPORT)


##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
