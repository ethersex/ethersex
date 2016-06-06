TOPDIR ?= ..
include $(TOPDIR)/.config

$(SOFT_UART_SUPPORT)_SRC += core/soft_uart.c
$(SOFT_SPI_SUPPORT)_SRC += core/soft_spi.c
$(USART_SUPPORT)_SRC += core/usart.c

ifneq ($(DEBUG_USE_SYSLOG),y)
ifneq ($(ARCH_HOST),y)
$(DEBUG)_SRC += core/debug.c
endif
endif

$(ARCH_AVR)_SRC += core/periodic.c
$(ARCH_AVR)_ECMD_SRC += core/periodic_ecmd.c
SRC += core/eeprom.c 
$(MBR_SUPPORT)_SRC += core/mbr.c

ifneq ($(USART_SPI_SUPPORT),y)
$(ARCH_AVR)_SRC += core/spi.c
else
$(ARCH_AVR)_SRC += core/spi_usart.c
endif

ifeq ($(ARCH_HOST),y)
CPPFLAGS += -Icore/host
endif

$(STATUSLED_HB_ACT_SUPPORT)_SRC += core/heartbeat.c

##############################################################################
# milliticks cruft
# depends on meta.m4, use y_AUTOGEN_SRC to avoid circular deps

$(ARCH_AVR)_AUTOGEN_SRC += core/periodic_milliticks.c

core/periodic_milliticks.c: meta.defines scripts/meta_periodic_milliticks.m4 meta.m4
	$(M4) `cat meta.defines` $(filter-out $<,$^) > $@

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

SUBSUBDIRS += core/crypto
SUBSUBDIRS += core/host
SUBSUBDIRS += core/portio
SUBSUBDIRS += core/scheduler
SUBSUBDIRS += core/tty
SUBSUBDIRS += core/gui
SUBSUBDIRS += core/util
SUBSUBDIRS += core/vfs
SUBSUBDIRS += core/crc
SUBSUBDIRS += core/queue
