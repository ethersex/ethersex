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

# flags for the compiler
CFLAGS += -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99

# flags for the linker
LDFLAGS += -mmcu=$(MCU)


##############################################################################
# the default target
all: $(TARGET)

##############################################################################
# include user's config.mk file

$(TOPDIR)/config.mk: 
	@echo "# Put your own config here!" > $@
	@echo "#F_CPU = $(F_CPU)" >> $@
	@echo "#MCU = $(MCU)" >> $@
	@echo "#LDFLAGS += -Wl,--section-start=.text=0xE000	# BOOTLOADER_SUPPORT" >> $@
	@echo "#CFLAGS  += -mcall-prologues                     # BOOTLOADER_SUPPORT" >> $@
	@echo "created default config.mk, tune your settings there!"
-include $(TOPDIR)/config.mk
