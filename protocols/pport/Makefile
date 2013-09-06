TOPDIR ?= ../..
include $(TOPDIR)/.config

# The order does matter, pport.c must be listed before pport_net.c because
# of meta call order!
$(PPORT_SUPPORT)_SRC += protocols/pport/pport.c protocols/pport/pport_net.c
$(DEBUG_PPORT)_ECMD_SRC += protocols/pport/pport_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
