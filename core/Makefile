TOPDIR ?= ..
include $(TOPDIR)/.config

$(SOFT_UART_SUPPORT)_SRC += core/soft_uart.c
$(USART_SUPPORT)_SRC += core/usart.c
$(DEBUG)_SRC += core/debug.c

SRC += core/eeprom.c core/periodic.c core/spi.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

