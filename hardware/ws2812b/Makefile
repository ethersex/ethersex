TOPDIR ?= ../..
include $(TOPDIR)/.config

$(WS2812B_SUPPORT)_ASRC += hardware/ws2812b/ws2812b.S
$(WS2812B_DYNAPI_SUPPORT)_ASRC += hardware/ws2812b/ws2812b_dyn.S

$(TOPDIR)/hardware/ws2812b/ws2812b.o: $(TOPDIR)/.config
$(TOPDIR)/hardware/ws2812b/ws2812b_dyn.o: $(TOPDIR)/.config

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
