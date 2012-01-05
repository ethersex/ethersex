TOPDIR ?= ../..
include $(TOPDIR)/.config

# The order does matter, ems.c must be listed before ems_net.c because
# of meta call order!
$(EMS_SUPPORT)_SRC +=            \
	protocols/ems/ems.c      \
	protocols/ems/ems_net.c  \
	protocols/ems/ems_uart.c \
	protocols/ems/ems_proto.c
$(EMS_DEBUG_STATS)_ECMD_SRC += protocols/ems/ems_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
