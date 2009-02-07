# ethersex project specific defaults, each can be overridden in config.mk
F_CPU = 20000000UL
MCU = atmega644

CC=avr-gcc
AR=avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 115200
SIZE = avr-size
STRIP = avr-strip

HOSTCC := gcc
export HOSTCC

# flags for the compiler
CPPFLAGS += -mmcu=$(MCU)  
CFLAGS ?= -Wall -W -Wno-unused-parameter -Wno-sign-compare
CFLAGS += -g -Os -std=gnu99

# flags for the linker
LDFLAGS += -mmcu=$(MCU)


##############################################################################
# the default target
$(TARGET):

##############################################################################
# include user's config.mk file

# By now we don't generate an empty example config.mk any longer,
# since must stuff can be controlled via menuconfig finally ...
-include $(TOPDIR)/config.mk

##############################################################################
# configure load address for bootloader, if enabled
#
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),mrproper)
ifneq ($(MAKECMDGOALS),menuconfig)  

include $(TOPDIR)/.config

CPPFLAGS += -DF_CPU=$(FREQ)UL -mmcu=$(MCU)

endif # MAKECMDGOALS!=menuconfig
endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean

ifeq ($(BOOTLOADER_SUPPORT),y)  
LDFLAGS += -Wl,--section-start=.text=0xE000
CFLAGS  += -mcall-prologues
endif


%.s: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -S $<

%.E: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -C -E -dD $<

