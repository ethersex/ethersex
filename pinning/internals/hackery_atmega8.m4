#define _ATMEGA8

/* ATmega8 specific adjustments */
#define _IVREG GICR
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _TIFR_TIMER1 TIFR
#define _TIMSK_TIMER1 TIMSK
#define _EIMSK GICR
#define USART0_UDRE_vect USART_UDRE_vect
#define USART0_RX_vect USART_RXC_vect
#define USART0_TX_vect USART_TXC_vect

#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _SIG_OUTPUT_COMPARE2 SIG_OUTPUT_COMPARE2
#define _SIG_OVERFLOW2 SIG_OVERFLOW2
#define _TIMSK_TIMER2 TIMSK

#define BOOTLOADER_SECTION 0x0E00 /* atmega8 with 256 words bootloader */


#define HD44780_CTRL_PORT D
#define HD44780_DATA_PORT D
#define HD44780_RS PD0
#define HD44780_EN PD1
#define HD44780_D4 PD4
#define HD44780_D5 PD5
#define HD44780_D6 PD6
#define HD44780_D7 PD7
#define HD44780_DATA_SHIFT 4

