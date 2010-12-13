
#define _ATMEGA168P
#define USART0_RX_vect USART_RX_vect
#define USART0_TX_vect USART_TX_vect

/* the 168p has different port-definitions */
/* define the ones used everywhere else */
/* Problem not fixed in avr libc */
#ifndef PB7
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
#endif

#ifndef SIG_INTERRUPT0
#define SIG_INTERRUPT0 INT0_vect
#define SIG_INTERRUPT1 INT1_vect
#endif

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

/* Generic timer macros */
#define TC0_PRESCALER_1      {TCCR0&=~(_BV(CS02)|_BV(CS01));TCCR0|=_BV(CS00);}
#define TC0_PRESCALER_8      {TCCR0&=~(_BV(CS02)|_BV(CS00));TCCR0|=_BV(CS01);}
#define TC0_PRESCALER_64     {TCCR0&=~(_BV(CS02));TCCR0|=_BV(CS01)|_BV(CS02);}
#define TC0_PRESCALER_256    {TCCR0&=~(_BV(CS01)|_BV(CS00));TCCR0|=_BV(CS02);}
#define TC0_PRESCALER_1024   {TCCR0&=~(_BV(CS01));TCCR0|=_BV(CS02)|_BV(CS00);}

#define TC0_MODE_OFF	     {TCCR0&=~(_BV(WGM01)|_BV(WGM00));}
#define TC0_MODE_PWM         {TCCR0&=~(_BV(WGM01);TCCR0|=_BV(WGM00);}
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

#define TC0_VECTOR_OVERFLOW  TIMER0_OVF_vect
#define TC0_VECTOR_COMPARE   TIMER0_COMP_vect

#define TC2_PRESCALER_1      {TCCR2&=~(_BV(CS22)|_BV(CS21));TCCR2|=_BV(CS20);}
#define TC2_PRESCALER_8      {TCCR2&=~(_BV(CS22)|_BV(CS20));TCCR2|=_BV(CS21);}
#define TC2_PRESCALER_64     {TCCR2&=~(_BV(CS22));TCCR2|=_BV(CS21)|_BV(CS22);}
#define TC2_PRESCALER_256    {TCCR2&=~(_BV(CS21)|_BV(CS20));TCCR2|=_BV(CS22);}
#define TC2_PRESCALER_1024   {TCCR2&=~(_BV(CS21));TCCR2|=_BV(CS22)|_BV(CS20);}

#define TC2_MODE_OFF	     {TCCR2&=~(_BV(WGM21)|_BV(WGM20));}
#define TC2_MODE_PWM         {TCCR2&=~(_BV(WGM21);TCCR2|=_BV(WGM20);}
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

#define TC2_VECTOR_OVERFLOW  TIMER2_OVF_vect
#define TC2_VECTOR_COMPARE   TIMER2_COMP_vect

/* Timer2 - Stella */
#define _TCCR2_PRESCALE TCCR2B
#define _OUTPUT_COMPARE_IE2 OCIE2B
#define _OUTPUT_COMPARE_REG2 OCR2B
#define _VECTOR_OUTPUT_COMPARE2 TIMER2_COMPB_vect
#define _VECTOR_OVERFLOW2 TIMER2_OVF_vect
#define _TIMSK_TIMER2 TIMSK2
