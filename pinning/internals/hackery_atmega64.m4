
#define _ATMEGA64

#define _SPMCR SPMCSR

/* ATmega64 specific adjustments */
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

/* i did not test the following stuff... cheers, ths. */
#define BOOTLOADER_SECTION 0xe000 /* atmega644 with 4096 words bootloader */

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

