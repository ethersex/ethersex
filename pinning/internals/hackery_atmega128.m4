
#define _ATMEGA128

#define _SPMCR SPMCSR

/* ATmega128 specific adjustments */
#define _TIMSK_TIMER0 TIMSK
#define _TIMSK_TIMER1 TIMSK
#define _IVREG MCUCR
#define _EIMSK EIMSK

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMP_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK

/* Timer0 - PWM Melody */
#define _PWM_MELODY_COMP TIMER0_COMP_vect
#define _PWM_MELODY_OCR OCR0
#define _PWM_MELODY_TRCCRA TCCR0
#define _PWM_MELODY_TRCCRB TCCR0
#define _PWM_MELODY_COM1 COM01
#define _PWM_MELODY_COM0 COM00
#define _PWM_MELODY_WGM0 WGM00
#define _PWM_MELODY_CS0 CS00
#define _PWM_MELODY_TIMSK TIMSK
#define _PWM_MELODY_OCIE OCIE0

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

#define _TIFR_TIMER0 TIFR
#define _TIFR_TIMER1 TIFR
#define _TIFR_TIMER2 TIFR

/* USART Register C (generic) */
#undef    UMSEL
#undef    UPM1
#undef    UPM0
#undef    USBS
#undef    UCSZ1
#undef    UCSZ0
#undef    UCPOL


/* USART Status Register A (generic) */
#undef    RXC
#undef    TXC
#undef    UDRE
#undef    FE
#undef    DOR
#undef    UPE
#undef    U2X
#undef    MPCM

/* USART Control Register B (generic) */
#undef    RXCIE
#undef    TXCIE
#undef    UDRIE
#undef    RXEN
#undef    TXEN
#undef    UCSZ
#undef    UCSZ2
#undef    RXB8
#undef    TXB8

