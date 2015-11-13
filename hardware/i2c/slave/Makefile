TOPDIR ?= ..
include $(TOPDIR)/.config

$(ECMD_TWI_SLAVE_SUPPORT)_TWISLAVE += hardware/i2c/slave/i2c_slave_ecmd.c
$(TWI_SLAVE_SUPPORT)_SRC += hardware/i2c/slave/i2c_slave.c \
	$(y_TWISLAVE) 

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
