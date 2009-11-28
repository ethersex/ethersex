TOPDIR ?= ..
include $(TOPDIR)/.config

$(SOFT_UART_SUPPORT)_SRC += core/soft_uart.c
$(USART_SUPPORT)_SRC += core/usart.c

ifneq ($(DEBUG_USE_SYSLOG),y)
ifneq ($(ARCH_HOST),y)
$(DEBUG)_SRC += core/debug.c
endif
endif

$(ARCH_AVR)_SRC += core/periodic.c
SRC += core/eeprom.c 

ifneq ($(USART_SPI_SUPPORT),y)
$(ARCH_AVR)_SRC += core/spi.c
else
$(ARCH_AVR)_SRC += core/spi_usart.c
endif

ifeq ($(ARCH_HOST),y)
CPPFLAGS += -Icore/host
endif

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

