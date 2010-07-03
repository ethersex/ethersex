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
#define _TIFR_TIMER2 TIFR
#define _TIMSK_TIMER1 TIMSK
#define _TIMSK_TIMER2 TIMSK
#define _EIMSK GICR
#define EIMSK GICR
#define EICRA MCUCR
#define _EICRA MCUCR
#define EIFR  GIFR
#define USART0_UDRE_vect USART_UDRE_vect
#define USART0_RX_vect USART_RXC_vect
#define USART0_TX_vect USART_TXC_vect

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMP_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK

#define RC5_USE_TIMER2

#define BOOTLOADER_SECTION 0x0E00 /* atmega8 with 256 words bootloader */
