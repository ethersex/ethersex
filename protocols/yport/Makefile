TOPDIR ?= ../..
include $(TOPDIR)/.config

# The order does matter, yport.c must be listed before yport_net.c because
# of meta call order!
$(YPORT_SUPPORT)_SRC += protocols/yport/yport.c protocols/yport/yport_net.c
$(DEBUG_YPORT)_ECMD_SRC += protocols/yport/yport_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
