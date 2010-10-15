
#define _ATMEGA644

#define _SPMCR SPMCSR

/* ATmega644 specific adjustments */
#define _IVREG MCUCR
#define _EIMSK EIMSK
#define _EICRA EICRA

/* Timer0 - ASK Sense */
#define _TCCR0_PRESCALE TCCR0B
#define _VECTOR_OVERFLOW0 TIMER0_OVF_vect
#define _TIMSK_TIMER0 TIMSK0

/* Timer0 - Stella */
#define _TCCR0_PRESCALE TCCR0B
#define _OUTPUT_COMPARE_IE0 OCIE0B
#define _OUTPUT_COMPARE_REG0 OCR0B
#define _VECTOR_OUTPUT_COMPARE0 TIMER0_COMPB_vect
#define _VECTOR_OVERFLOW0 TIMER0_OVF_vect
#define _TIFR_TIMER0 TIFR0

/* Timer1 - Ticks */
#define _TIMSK_TIMER1 TIMSK1

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2B
#define _OUTPUT_COMPARE_IE2 OCIE2B
#define _OUTPUT_COMPARE_REG2 OCR2B
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMPB_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK2
#define _TIFR_TIMER2 TIFR2

/* Timer0 - PWM Melody */
#define _PWM_MELODY_COMP TIMER2_COMPA_vect
#define _PWM_MELODY_OCR OCR2A
#define _PWM_MELODY_TRCCRA TCCR2A
#define _PWM_MELODY_TRCCRB TCCR2B
#define _PWM_MELODY_COM1 COM2A1
#define _PWM_MELODY_COM0 COM2B0
#define _PWM_MELODY_WGM0 WGM20
#define _PWM_MELODY_CS0 CS20
#define _PWM_MELODY_TIMSK TIMSK2
#define _PWM_MELODY_OCIE OCIE2A

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

#define _TIFR_TIMER1 TIFR1
