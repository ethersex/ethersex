#define _ATMEGA32

#define _SPMCR SPMCR

/* ATmega32 specific adjustments */
#define USART0_UDRE_vect USART_UDRE_vect
#define USART0_RX_vect USART_RXC_vect
#define USART0_TX_vect USART_TXC_vect
#define UPE PE /* parity error */
#define _IVREG MCUCR
#define _EIMSK GICR
#define _EICRA MCUCR

/* Generic timer macros */
#define TC0_PRESCALER_1      {TCCR0&=~(_BV(CS02)|_BV(CS01));TCCR0|=_BV(CS00);}
#define TC0_PRESCALER_8      {TCCR0&=~(_BV(CS02)|_BV(CS00));TCCR0|=_BV(CS01);}
#define TC0_PRESCALER_64     {TCCR0&=~(_BV(CS02));TCCR0|=_BV(CS01)|_BV(CS02);}
#define TC0_PRESCALER_256    {TCCR0&=~(_BV(CS01)|_BV(CS00));TCCR0|=_BV(CS02);}
#define TC0_PRESCALER_1024   {TCCR0&=~(_BV(CS01));TCCR0|=_BV(CS02)|_BV(CS00);}

#define TC0_MODE_OFF	     {TCCR0&=~(_BV(WGM01)|_BV(WGM00));}
#define TC0_MODE_PWM         {TCCR0&=~(_BV(WGM01));TCCR0|=_BV(WGM00);}
#define TC0_MODE_CTC         {TCCR0&=~(_BV(WGM00));TCCR0|=_BV(WGM01);}
#define TC0_MODE_PWMFAST     {TCCR0|=_BV(WGM01)|_BV(WGM00);}

#define TC0_OUTPUT_COMPARE_NONE   {TCCR0&=~(_BV(COM01)|_BV(COM00));}
#define TC0_OUTPUT_COMPARE_TOGGLE {TCCR0&=~(_BV(COM01));TCCR0|=_BV(COM00);}
#define TC0_OUTPUT_COMPARE_CLEAR  {TCCR0&=~(_BV(COM00));TCCR0|=_BV(COM01);}
#define TC0_OUTPUT_COMPARE_SET    {TCCR0|=_BV(COM01)|_BV(COM00);}

#define TC0_COUNTER_CURRENT  TCNT0
#define TC0_COUNTER_COMPARE  OCR0

#define TC0_INT_COMPARE_ON   TIMSK|=_BV(OCIE0);
#define TC0_INT_COMPARE_OFF  TIMSK&=~_BV(OCIE0);
#define TC0_INT_OVERFLOW_ON  TIMSK|=_BV(TOIE0);
#define TC0_INT_OVERFLOW_OFF TIMSK&=~_BV(TOIE0);

#define TC0_INT_OVERFLOW_TST (TIFR&_BV(TOV0))
#define TC0_INT_OVERFLOW_CLR TIFR=_BV(TOV0);

#define TC0_VECTOR_OVERFLOW  TIMER0_OVF_vect
#define TC0_VECTOR_COMPARE   TIMER0_COMP_vect

#define TC2_PRESCALER_1      {TCCR2&=~(_BV(CS22)|_BV(CS21));TCCR2|=_BV(CS20);}
#define TC2_PRESCALER_8      {TCCR2&=~(_BV(CS22)|_BV(CS20));TCCR2|=_BV(CS21);}
#define TC2_PRESCALER_64     {TCCR2&=~(_BV(CS22));TCCR2|=_BV(CS21)|_BV(CS22);}
#define TC2_PRESCALER_256    {TCCR2&=~(_BV(CS21)|_BV(CS20));TCCR2|=_BV(CS22);}
#define TC2_PRESCALER_1024   {TCCR2&=~(_BV(CS21));TCCR2|=_BV(CS22)|_BV(CS20);}

#define TC2_MODE_OFF	     {TCCR2&=~(_BV(WGM21)|_BV(WGM20));}
#define TC2_MODE_PWM         {TCCR2&=~(_BV(WGM21));TCCR2|=_BV(WGM20);}
#define TC2_MODE_CTC         {TCCR2&=~(_BV(WGM20));TCCR2|=_BV(WGM21);}
#define TC2_MODE_PWMFAST     {TCCR2|=_BV(WGM21)|_BV(WGM20);}

#define TC2_OUTPUT_COMPARE_NONE   {TCCR2&=~(_BV(COM21)|_BV(COM20));}
#define TC2_OUTPUT_COMPARE_TOGGLE {TCCR2&=~(_BV(COM21));TCCR2|=_BV(COM20);}
#define TC2_OUTPUT_COMPARE_CLEAR  {TCCR2&=~(_BV(COM20));TCCR2|=_BV(COM21);}
#define TC2_OUTPUT_COMPARE_SET    {TCCR2|=_BV(COM21)|_BV(COM20);}

#define TC2_COUNTER_CURRENT  TCNT2
#define TC2_COUNTER_COMPARE  OCR2

#define TC2_INT_COMPARE_ON   TIMSK|=_BV(OCIE2);
#define TC2_INT_COMPARE_OFF  TIMSK&=~_BV(OCIE2);
#define TC2_INT_OVERFLOW_ON  TIMSK|=_BV(TOIE2);
#define TC2_INT_OVERFLOW_OFF TIMSK&=~_BV(TOIE2);

#define TC2_INT_OVERFLOW_TST (TIFR&_BV(TOV2))
#define TC2_INT_OVERFLOW_CLR TIFR=_BV(TOV2);

#define TC2_VECTOR_OVERFLOW  TIMER2_OVF_vect
#define TC2_VECTOR_COMPARE   TIMER2_COMP_vect

/* Timer1 - Ticks */
#define _TIMSK_TIMER1 TIMSK

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMP_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK
#define TCR2BUB TCR2UB
#define _CS20 CS20
#define _CS21 CS21
#define _CS22 CS22
#define _COM20 COM20
#define _COM21 COM21
#define _WGM20 WGM20
#define _WGM21 WGM21
#define _TCNT2 TCNT2

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

