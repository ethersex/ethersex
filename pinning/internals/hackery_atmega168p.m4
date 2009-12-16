
#define BOOTLOADER_SECTION 0x0c00 /* atmega168p with 1024 words bootloader */

#define _ATMEGA168P
#define USART0_RX_vect USART_RX_vect
#define USART0_TX_vect USART_TX_vect

/* the 168p has different port-definitions */
/* define the ones used everywhere else */
#define PB7     PORTB7
#define PB6     PORTB6
#define PB5     PORTB5
#define PB4     PORTB4
#define PB3     PORTB3
#define PB2     PORTB2
#define PB1     PORTB1
#define PB0     PORTB0

#define PC6     PORTC6
#define PC5     PORTC5
#define PC4     PORTC4
#define PC3     PORTC3
#define PC2     PORTC2
#define PC1     PORTC1
#define PC0     PORTC0

#define PD7     PORTD7
#define PD6     PORTD6
#define PD5     PORTD5
#define PD4     PORTD4
#define PD3     PORTD3
#define PD2     PORTD2
#define PD1     PORTD1
#define PD0     PORTD0

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

#define _TIMSK_TIMER1 TIMSK1

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2B
#define _OUTPUT_COMPARE_IE2 OCIE2B
#define _OUTPUT_COMPARE_REG2 OCR2B
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMPB_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK2
