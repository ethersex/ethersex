# ethersex project specific defaults, each can be overridden in config.mk
F_CPU = 20000000UL
MCU = atmega644

CP = cp
RM = rm -f
AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 115200
AWK = gawk
M4 = m4

### use GNU sed from macports instead of BSD sed on MacOS X
SED = $(shell if [ $$OSTYPE = "darwin10.0" ]; then echo gsed; \
	else echo sed; fi)

HOSTCC := gcc
export HOSTCC
export MAKE

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

CPPFLAGS += -DF_CPU=$(FREQ)UL

endif # MAKECMDGOALS!=menuconfig
endif # MAKECMDGOALS!=mrproper
endif # MAKECMDGOALS!=clean

CFLAGS ?= -Wall -W -Wno-unused-parameter -Wno-sign-compare

ifeq ($(ARCH_HOST),y)
  CC=gcc
  AR=ar
  OBJCOPY = objcopy
  OBJDUMP = objdump
  AS = as
  SIZE = size
  STRIP = strip

  CPPFLAGS += -I$(TOPDIR)
  CFLAGS += -ggdb -O0 -std=gnu99

else

  CC=avr-gcc
  AR=avr-ar
  OBJCOPY = avr-objcopy
  OBJDUMP = avr-objdump
  AS = avr-as
  SIZE = avr-size
  STRIP = avr-strip

  # flags for the compiler
  CPPFLAGS += -mmcu=$(MCU) -I$(TOPDIR)
  CFLAGS += -g -Os -std=gnu99

  # flags for the linker
  LDFLAGS += -mmcu=$(MCU)

endif

# remove all unused code and data during linking
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections

ifeq ($(BOOTLOADER_SUPPORT),y)
ifeq ($(atmega128),y)
LDFLAGS += -Wl,--section-start=.text=0x1E000
else
LDFLAGS += -Wl,--section-start=.text=0xE000
endif

CFLAGS  += -mcall-prologues
endif


%.s: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -std=gnu99 -S $<

%.E: %.c
	$(CC) -o $@ -O0 $(CPPFLAGS) -C -E -dD $<

%.o: %.S
	$(CC) -o $@ $(CPPFLAGS) $(ASFLAGS) -c $<
