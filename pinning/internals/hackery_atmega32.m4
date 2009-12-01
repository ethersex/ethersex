#define _ATMEGA32

#define _SPMCR SPMCR

/* ATmega32 specific adjustments */
#define USART0_UDRE_vect USART_UDRE_vect
#define USART0_RX_vect USART_RXC_vect
#define USART0_TX_vect USART_TXC_vect
#define _IVREG MCUCR
#define _EIMSK GICR
#define _EICRA MCUCR

/* Timer0 - ASK Sense */
#define _TCCR0_PRESCALE TCCR0
#define _VECTOR_OVERFLOW0 TIMER0_OVF_vect
#define _TIMSK_TIMER0 TIMSK

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMP_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK
#define _TIFR_TIMER2 TIFR
#define TCR2BUB TCR2UB

/* Timer2 - PWM Melody */
#define _PWM_MELODY_COMP TIMER2_COMP_vect
#define _PWM_MELODY_OCR OCR2
#define _PWM_MELODY_TRCCRA TCCR2
#define _PWM_MELODY_TRCCRB TCCR2
#define _PWM_MELODY_COM1 COM21
#define _PWM_MELODY_COM0 COM20
#define _PWM_MELODY_WGM0 WGM20
#define _PWM_MELODY_CS0 CS20
#define _PWM_MELODY_TIMSK TIMSK
#define _PWM_MELODY_OCIE OCIE2

/* workaround for avr-libc devs not being able to decide how these registers
 * should be named... */
#ifdef SPCR0
    #define _SPCR0 SPCR0
#else
    #define _SPCR0 SPCR
#endif

#ifdef SPE0
    #define _SPE0 SPE0
#else
    #define _SPE0 SPE
#endif

#ifdef MSTR0
    #define _MSTR0 MSTR0
#else
    #define _MSTR0 MSTR
#endif

#ifdef SPSR0
    #define _SPSR0 SPSR0
#else
    #define _SPSR0 SPSR
#endif

#ifdef SPIF0
    #define _SPIF0 SPIF0
#else
    #define _SPIF0 SPIF
#endif

#ifdef SPDR0
    #define _SPDR0 SPDR0
#else
    #define _SPDR0 SPDR
#endif

#ifdef SPI2X0
    #define _SPI2X0 SPI2X0
#else
    #define _SPI2X0 SPI2X
#endif

#define _TIFR_TIMER1 TIFR

dnl don't know if it is the right value
#define BOOTLOADER_SECTION 0xe000 /* atmega32 with 4096 words bootloader */
