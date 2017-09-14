dnl
dnl hackery_atmega32u4.m4
dnl
dnl   Copyright (c) 2017 by Erik Kunze <ethersex@erik-kunze.de>
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by
dnl   the Free Software Foundation; either version 3 of the License, or
dnl   (at your option) any later version.
dnl
dnl
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl


#define _ATMEGA32U4

#define _SPMCR SPMCSR

/* ATmega32u4 specific adjustments */
//#define _EIMSK EIMSK
//#define _IVREG MCUCR

/* Watchdog status register */
#define MCU_STATUS_REGISTER  MCUSR

/* Generic timer macros */
#define TC0_PRESCALER_1      {TCCR0B&=~(_BV(CS02)|_BV(CS01));TCCR0B|=_BV(CS00);}
#define TC0_PRESCALER_8      {TCCR0B&=~(_BV(CS02)|_BV(CS00));TCCR0B|=_BV(CS01);}
#define TC0_PRESCALER_64     {TCCR0B&=~(_BV(CS02));TCCR0B|=_BV(CS01)|_BV(CS00);}
#define TC0_PRESCALER_256    {TCCR0B&=~(_BV(CS01)|_BV(CS00));TCCR0B|=_BV(CS02);}
#define TC0_PRESCALER_1024   {TCCR0B&=~(_BV(CS01));TCCR0B|=_BV(CS02)|_BV(CS00);}

#define TC0_MODE_OFF         {TCCR0A&=~(_BV(WGM01)|_BV(WGM00));TCCR0B&=~(_BV(WGM02));}
#define TC0_MODE_PWM         {TCCR0A&=~(_BV(WGM01));TCCR0A|=_BV(WGM00);TCCR0B&=~(_BV(WGM02));}
#define TC0_MODE_CTC         {TCCR0A&=~(_BV(WGM00));TCCR0A|=_BV(WGM01);TCCR0B&=~(_BV(WGM02));}
#define TC0_MODE_PWMFAST     {TCCR0A|=_BV(WGM01)|_BV(WGM00);TCCR0B&=~(_BV(WGM02));}

#define TC0_OUTPUT_COMPARE_NONE   {TCCR0A&=~(_BV(COM0A1)|_BV(COM0A0));}
#define TC0_OUTPUT_COMPARE_TOGGLE {TCCR0A&=~(_BV(COM0A1));TCCR0A|=_BV(COM0A0);}
#define TC0_OUTPUT_COMPARE_CLEAR  {TCCR0A&=~(_BV(COM0A0));TCCR0A|=_BV(COM0A1);}
#define TC0_OUTPUT_COMPARE_SET    {TCCR0A|=_BV(COM0A1)|_BV(COM0A0);}

#define TC0_COUNTER_CURRENT  TCNT0
#define TC0_COUNTER_COMPARE  OCR0A

#define TC0_INT_COMPARE_ON   TIMSK0|=_BV(OCIE0A);
#define TC0_INT_COMPARE_OFF  TIMSK0&=~_BV(OCIE0A);
#define TC0_INT_OVERFLOW_ON  TIMSK0|=_BV(TOIE0);
#define TC0_INT_OVERFLOW_OFF TIMSK0&=~_BV(TOIE0);

#define TC0_VECTOR_OVERFLOW  TIMER0_OVF_vect
#define TC0_VECTOR_COMPARE   TIMER0_COMPA_vect

#define TC0_INT_COMPARE_TST  (TIFR0&_BV(OCF0))
#define TC0_INT_COMPARE_CLR  TIFR0=_BV(OCF0);
#define TC0_INT_OVERFLOW_TST (TIFR0&_BV(TOV0))
#define TC0_INT_OVERFLOW_CLR TIFR0=_BV(TOV0);

#define TC1_PRESCALER_1      {TCCR1B&=~(_BV(CS12)|_BV(CS11));TCCR1B|=_BV(CS10);}
#define TC1_PRESCALER_8      {TCCR1B&=~(_BV(CS12)|_BV(CS10));TCCR1B|=_BV(CS11);}
#define TC1_PRESCALER_64     {TCCR1B&=~(_BV(CS12));TCCR1B|=_BV(CS11)|_BV(CS10);}
#define TC1_PRESCALER_256    {TCCR1B&=~(_BV(CS11)|_BV(CS10));TCCR1B|=_BV(CS12);}
#define TC1_PRESCALER_1024   {TCCR1B&=~(_BV(CS11));TCCR1B|=_BV(CS12)|_BV(CS10);}

#define TC1_MODE_OFF         {TCCR1A&=~(_BV(WGM11)|_BV(WGM10));TCCR1B&=~(_BV(WGM12)|_BV(WGM13));}
#define TC1_MODE_PWM         {TCCR1A&=~(_BV(WGM11));TCCR1A|=_BV(WGM10);TCCR1B&=~(_BV(WGM12)|_BV(WGM13));}
#define TC1_MODE_CTC         {TCCR1A&=~(_BV(WGM11)|_BV(WGM10));TCCR1B|=_BV(WGM12);TCCR1B&=~(_BV(WGM13));}
#define TC1_MODE_PWMFAST     {TCCR1A|=_BV(WGM11)|_BV(WGM10);TCCR1B|=_BV(WGM12);TCCR1B&=~(_BV(WGM13));}
#define TC1_MODE_PWMFAST_OCR {TCCR1A|=_BV(WGM11)|_BV(WGM10);TCCR1B|=_BV(WGM12)|_BV(WGM13);}

#define TC1_OUTPUT_COMPARE_NONE   {TCCR1A&=~(_BV(COM1A1)|_BV(COM1A0));}
#define TC1_OUTPUT_COMPARE_TOGGLE {TCCR1A&=~(_BV(COM1A1));TCCR1A|=_BV(COM1A0);}
#define TC1_OUTPUT_COMPARE_CLEAR  {TCCR1A&=~(_BV(COM1A0));TCCR1A|=_BV(COM1A1);}
#define TC1_OUTPUT_COMPARE_SET    {TCCR1A|=_BV(COM1A1)|_BV(COM1A0);}

#define TC1_COUNTER_CURRENT  TCNT1
#define TC1_COUNTER_COMPARE  OCR1A

#define TC1_INT_COMPARE_ON   TIMSK1|=_BV(OCIE1A);
#define TC1_INT_COMPARE_OFF  TIMSK1&=~_BV(OCIE1A);
#define TC1_INT_OVERFLOW_ON  TIMSK1|=_BV(TOIE1);
#define TC1_INT_OVERFLOW_OFF TIMSK1&=~_BV(TOIE1);

#define TC1_INT_OVERFLOW_TST (TIFR1&_BV(TOV1))
#define TC1_INT_OVERFLOW_CLR TIFR1=_BV(TOV1);

#define TC1_VECTOR_OVERFLOW  TIMER1_OVF_vect
#define TC1_VECTOR_COMPARE   TIMER1_COMPA_vect

#define TC3_PRESCALER_1      {TCCR3B&=~(_BV(CS32)|_BV(CS31));TCCR3B|=_BV(CS30);}
#define TC3_PRESCALER_8      {TCCR3B&=~(_BV(CS32)|_BV(CS30));TCCR3B|=_BV(CS31);}
#define TC3_PRESCALER_64     {TCCR3B&=~(_BV(CS32));TCCR3B|=_BV(CS31)|_BV(CS30);}
#define TC3_PRESCALER_256    {TCCR3B&=~(_BV(CS31)|_BV(CS30));TCCR3B|=_BV(CS32);}
#define TC3_PRESCALER_1024   {TCCR3B&=~(_BV(CS31));TCCR3B|=_BV(CS32)|_BV(CS30);}

#define TC3_MODE_OFF         {TCCR3A&=~(_BV(WGM31)|_BV(WGM30));TCCR3B&=~(_BV(WGM32)|_BV(WGM33));}
#define TC3_MODE_PWM         {TCCR3A&=~(_BV(WGM31));TCCR3A|=_BV(WGM30);TCCR3B&=~(_BV(WGM32)|_BV(WGM33));}
#define TC3_MODE_CTC         {TCCR3A&=~(_BV(WGM31)|_BV(WGM30));TCCR3B|=_BV(WGM32);TCCR3B&=~(_BV(WGM33));}
#define TC3_MODE_PWMFAST     {TCCR3A|=_BV(WGM31)|_BV(WGM30);TCCR3B|=_BV(WGM32);TCCR3B&=~(_BV(WGM33));}
#define TC3_MODE_PWMFAST_OCR {TCCR3A|=_BV(WGM31)|_BV(WGM30);TCCR3B|=_BV(WGM32)|_BV(WGM33);}

#define TC3_OUTPUT_COMPARE_NONE   {TCCR3A&=~(_BV(COM3A1)|_BV(COM3A0));}
#define TC3_OUTPUT_COMPARE_TOGGLE {TCCR3A&=~(_BV(COM3A1));TCCR3A|=_BV(COM3A0);}
#define TC3_OUTPUT_COMPARE_CLEAR  {TCCR3A&=~(_BV(COM3A0));TCCR3A|=_BV(COM3A1);}
#define TC3_OUTPUT_COMPARE_SET    {TCCR3A|=_BV(COM3A1)|_BV(COM3A0);}

#define TC3_COUNTER_CURRENT  TCNT3
#define TC3_COUNTER_COMPARE  OCR3A

#define TC3_INT_COMPARE_ON   TIMSK3|=_BV(OCIE3A);
#define TC3_INT_COMPARE_OFF  TIMSK3&=~_BV(OCIE3A);
#define TC3_INT_OVERFLOW_ON  TIMSK3|=_BV(TOIE3);
#define TC3_INT_OVERFLOW_OFF TIMSK3&=~_BV(TOIE3);

#define TC3_INT_OVERFLOW_TST (TIFR3&_BV(TOV3))
#define TC3_INT_OVERFLOW_CLR TIFR3=_BV(TOV3);

#define TC3_VECTOR_OVERFLOW  TIMER3_OVF_vect
#define TC3_VECTOR_COMPARE   TIMER3_COMPA_vect

#define RXD1_PORT  D
#define RXD1_PIN   2

#define TXD1_PORT  D
#define TXD1_PIN   3

