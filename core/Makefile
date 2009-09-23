TOPDIR ?= ..
include $(TOPDIR)/.config

$(SOFT_UART_SUPPORT)_SRC += core/soft_uart.c
$(USART_SUPPORT)_SRC += core/usart.c

ifneq ($(DEBUG_USE_SYSLOG),y)
$(DEBUG)_SRC += core/debug.c
endif

ifneq ($(USART_SPI_SUPPORT),y)
SRC += core/spi.c
else
SRC += core/spi_usart.c
endif

SRC += core/eeprom.c core/periodic.c 

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

