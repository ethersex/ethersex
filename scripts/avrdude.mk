#
# One may override values from Menuconfig or add some unsupported options here
#
# Programming hardware
# Type: avrdude -c ?
# to get a full listing.
#
# AVRDUDE_PROGRAMMER = stk500v2
# AVRDUDE_PROGRAMMER = avrispmkII
# AVRDUDE_PROGRAMMER = avrisp2
# AVRDUDE_PROGRAMMER = avr911
# AVRDUDE_PROGRAMMER = jtag2
# AVRDUDE_PROGRAMMER = dragon_isp

# AVRDUDE_PORT = usb # Device discovery for avrispmkII
# AVRDUDE_PORT = /dev/ttyUSB0 # programmer connected to USB on Unix systems

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep

# Fuse settings
AVRDUDE_FUSE = -i 15 -B 4   # Slow down the writing

#ifeq ($(MCU),atmega1284p)
#AVRDUDE_LFUSE = 0xe6
#AVRDUDE_HFUSE = 0xd1
#AVRDUDE_EFUSE = 0xfd
#endif

# Uncomment the following and set to a sensible value to speed things up.
# Programming bitclock in microseconds, should be lower than 1/4 MCU clock.
#AVRDUDE_BITCLOCK = -B 0.3

# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
# AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude> 
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

####################
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(subst ",,$(AVRDUDE_BITCLOCK))
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(subst ",,$(AVRDUDE_VERBOSE))
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += $(subst ",,$(AVRDUDE_EXTRA_OPTIONS))

# Commands for fuse writing
AVRDUDE_WRITE_EFUSE = -U efuse:w:$(AVRDUDE_EFUSE):m
AVRDUDE_WRITE_HFUSE = -U hfuse:w:$(AVRDUDE_HFUSE):m
AVRDUDE_WRITE_LFUSE = -U lfuse:w:$(AVRDUDE_LFUSE):m

# Program the device.  
program: $(TARGET).hex #$(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)

# Write the fuses
fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_FUSE) $(AVRDUDE_WRITE_EFUSE) $(AVRDUDE_WRITE_HFUSE) $(AVRDUDE_WRITE_LFUSE)

	
