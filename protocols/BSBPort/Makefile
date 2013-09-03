TOPDIR ?= ../..
include $(TOPDIR)/.config

# The order does matter, bsbport.c must be listed before bsbport_net.c because
# of meta call order!
$(BSBPORT_SUPPORT)_SRC += protocols/BSBPort/bsbport.c protocols/BSBPort/bsbport_net.c protocols/BSBPort/bsbport_helper.c protocols/BSBPort/bsbport_rx.c protocols/BSBPort/bsbport_tx.c
$(BSBPORT_SUPPORT)_ECMD_SRC += protocols/BSBPort/bsbport_ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
