
#define BOOTLOADER_SECTION 0x0c00 /* atmega168 with 1024 words bootloader */

#define _ATMEGA168

/* ATmega168 specific adjustments */
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _TIFR_TIMER1 TIFR1
#define _EIMSK EIMSK
#define _IVREG MCUCR

