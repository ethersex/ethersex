/*
* frequency counter
* main measurement functions
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "core/periodic.h"

#include "freqcount.h"
#include "freqcount_internal.h"

// define them static to allow the compiler inlining them
static void start_measure(void);
static freqcount_average_state_t measure_done(void);
static uint8_t check_measure_timeout(void);
static void switch_channel_multiplex(void);

// variables to communicate between pin change ISR and control
volatile tick24bit_t freqcount_start;
volatile tick24bit_t freqcount_freq_end;
#ifdef FREQCOUNT_DUTY_SUPPORT
volatile tick24bit_t freqcount_on_end;
#endif
volatile freqcount_state_t freqcount_state = FC_DISABLED;

#if FREQCOUNT_CHANNELS > 1
uint8_t freqcount_current_channel;
#endif

void freqcount_init_measure(void)
{
    for (uint8_t i=0; i<FREQCOUNT_CHANNELS; i++)
    {
#ifdef FREQCOUNT_BOOT_ON
        freqcount_set_state(1,i);
#else
        freqcount_set_state(0,i);
#endif
    }
    
    // make sure FREQCOUNT_PIN is defined and an input
    DDR_CONFIG_IN(FREQCOUNT_PIN);

#if FREQCOUNT_CHANNELS > 1
    freqcount_current_channel_store=0;
    DDR_CONFIG_OUT(FREQCOUNT_CHANNEL_MULTIPLEX_BIT1);
#endif

#if FREQCOUNT_CHANNELS > 2
    DDR_CONFIG_OUT(FREQCOUNT_CHANNEL_MULTIPLEX_BIT2);
#endif
#if FREQCOUNT_CHANNELS > 4
    DDR_CONFIG_OUT(FREQCOUNT_CHANNEL_MULTIPLEX_BIT3);
#endif
#if FREQCOUNT_CHANNELS > 8
    DDR_CONFIG_OUT(FREQCOUNT_CHANNEL_MULTIPLEX_CS_A);
    PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_CS_A);
    DDR_CONFIG_OUT(FREQCOUNT_CHANNEL_MULTIPLEX_CS_B);
    PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_CS_B);
#endif
}

void freqcount_mainloop(void)
{
    if (freqcount_state>=FC_DISABLED)
    {
#if FREQCOUNT_CHANNELS > 1
        // look for the next enabled channel
        while (!freqcount_perchannel_data[freqcount_current_channel].enabled &&
            freqcount_current_channel_store < FREQCOUNT_CHANNELS)
        {
            freqcount_current_channel_store++;
        }
        
        if (freqcount_current_channel_store==FREQCOUNT_CHANNELS)
        {
            // no active channel found so wrap around
            freqcount_current_channel_store=0;
        }
        else
        {
            start_measure();
        }
#else
        if (freqcount_perchannel_data[freqcount_current_channel].enabled)
            start_measure();
#endif
    }
    else if (freqcount_state>=FC_DONE && freqcount_state<=FC_DONE_OVERFLOW4)
    {
        if (measure_done() == FC_AVERAGE_DONE)
        {
#if FREQCOUNT_CHANNELS > 1
            // this channel is done, in the next loop try the next channel
            freqcount_current_channel_store++;
#endif
        }
    }
    
    if (check_measure_timeout())
    {
#if FREQCOUNT_CHANNELS > 1
        // this channel is done, in the next loop try the next channel
        freqcount_current_channel_store++;
#endif
    }
}

static void start_measure(void)
{
    freqcount_state=FC_BEFORE_START;
    
    switch_channel_multiplex();
    
    // trigger on rising edge
    TCCR1B |= _BV(ICES1);

    // clear timer interrupt capture flag by writing a 1:
    // required after changing the trigger direction
    TIFR1 = _BV(ICF1);

    // disable the noise canceler
    TCCR1B &= ~(_BV(ICNC1));

    // disable triggering timer interrupt capture from the analog comparator
    // this makes sure the ICP1 pin is used
    ACSR &= ~(_BV(ACIC));
    
    // enable timer input capture interrupt
    TIMSK1 |= _BV(ICIE1);
}

static freqcount_average_state_t measure_done(void)
{
    // disable timer input capture interrupt
    TIMSK1 &= ~(_BV(ICIE1));
    freqcount_state=FC_DISABLED;
    
    // convert from 24 bit to 32 to do the math
    
    uint32_t t1=0, t2=0;
 
#ifndef FREQCOUNT_NOSLOW_SUPPORT
    *(((uint8_t*)(&t1))+2)=freqcount_start.high;
    *(((uint8_t*)(&t2))+2)=freqcount_freq_end.high;
#endif    
    *((uint16_t*)(&t1))=freqcount_start.low;
    *((uint16_t*)(&t2))=freqcount_freq_end.low;

    // handle overflow with different sized types
    if (t1 > t2)
    {
#ifdef FREQCOUNT_NOSLOW_SUPPORT
        *(((uint8_t*)(&t2))+2)=1;
#else
        *(((uint8_t*)(&t2))+3)=1;
#endif
    }
    
    // calc frequency
    uint32_t freqcount_ticks=t2-t1;
    
#ifdef FREQCOUNT_DUTY_SUPPORT
    // calc on cycle tickts
    // end of frequency measurement is start of on-cyle
#ifndef FREQCOUNT_NOSLOW_SUPPORT
    *(((uint8_t*)(&t1))+2)=freqcount_on_end.high;
#endif    
    *((uint16_t*)(&t1))=freqcount_on_end.low;

    // handle overflow with different sized types
    if (t2 > t1)
    {
#ifdef FREQCOUNT_NOSLOW_SUPPORT
        *(((uint8_t*)(&t1))+2)=1;
#else
        *(((uint8_t*)(&t1))+3)=1;
#endif
    }
    
    uint8_t freqcount_duty=((t1-t2)<<8)/freqcount_ticks;

    return freqcount_average_results(freqcount_ticks, freqcount_duty);
#else // FREQCOUNT_DUTY_SUPPORT
    return freqcount_average_results(freqcount_ticks);
#endif // FREQCOUNT_DUTY_SUPPORT
}

static uint8_t check_measure_timeout(void)
{
    if(freqcount_state>FC_DISABLED)
    {
        // make sure freqcount_state does not overflow when not in use
        // because timer_overflow() keeps increasing it
        freqcount_state=FC_DISABLED;
        return 0;
    }
    else if ((freqcount_state>=FC_BEFORE_START_OVERFLOW2 && freqcount_state<=FC_BEFORE_START_OVERFLOW4) ||
             (freqcount_state>=FC_FREQ_OVERFLOW2 && freqcount_state<=FC_FREQ_OVERFLOW4) ||
             (freqcount_state>=FC_ON_CYCLE_OVERFLOW2 && freqcount_state<=FC_ON_CYCLE_OVERFLOW4))
    {
        // we are inside a measurement, but we had at least two overflows
        // -> we can't get reliable data anymore

        // disable timer input capture interrupt
        TIMSK1 &= ~(_BV(ICIE1));

        freqcount_state=FC_DISABLED;
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_average_results(0,0);
#else
        freqcount_average_results(0);
#endif
        return 1;
    }
    
    return 0;
}

ISR(TIMER1_CAPT_vect)
{
    if (freqcount_state<=FC_BEFORE_START_OVERFLOW4)
    {
        // FC_BEFORE_START: start to measure frequency
        freqcount_start.low=ICR1;
#ifndef FREQCOUNT_NOSLOW_SUPPORT
        freqcount_start.high=timer_overflows;
#endif
        freqcount_state=FC_FREQ;
    }
    else if (freqcount_state<=FC_FREQ_OVERFLOW4)
    {
        // state FC_FREQ, next rising edge -> we have the frequency
        freqcount_freq_end.low=ICR1;
#ifndef FREQCOUNT_NOSLOW_SUPPORT
        freqcount_freq_end.high=timer_overflows;
#endif

#ifndef FREQCOUNT_DUTY_SUPPORT
        freqcount_state=FC_DONE;
#else /* FREQCOUNT_DUTY_SUPPORT */
        // start to measure the on-cycle
        // trigger on falling edge
        TCCR1B &= ~(_BV(ICES1));
        
        // clear timer interrupt capture flag by writing a 1:
        // required after changing the trigger direction
        TIFR1 = _BV(ICF1);
        
        freqcount_state=FC_ON_CYCLE;
    }
    else if (freqcount_state<=FC_ON_CYCLE_OVERFLOW4)
    {
        // FC_ON_CYCLE, next falling edge: we have the on-time
        freqcount_on_end.low=ICR1;
#ifndef FREQCOUNT_NOSLOW_SUPPORT
        freqcount_on_end.high=timer_overflows;
#endif /* FREQCOUNT_NOSLOW_SUPPORT */
        freqcount_state=FC_DONE;
#endif /* FREQCOUNT_DUTY_SUPPORT */
    }
}

static void switch_channel_multiplex(void)
{
#if FREQCOUNT_CHANNELS > 1
    if (freqcount_current_channel & 0x01)
        PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_BIT1);
    else
        PIN_CLEAR(FREQCOUNT_CHANNEL_MULTIPLEX_BIT1);
#endif

#if FREQCOUNT_CHANNELS > 2
    if (freqcount_current_channel & 0x02)
        PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_BIT2);
    else
        PIN_CLEAR(FREQCOUNT_CHANNEL_MULTIPLEX_BIT2);
#endif

#if FREQCOUNT_CHANNELS > 4
    if (freqcount_current_channel & 0x04)
        PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_BIT3);
    else
        PIN_CLEAR(FREQCOUNT_CHANNEL_MULTIPLEX_BIT3);
#endif

#if FREQCOUNT_CHANNELS > 8
    // we have two 74xx251 multiplexers
    // they are enabled by setting the respective
    // chip select line to low
    // otherwise their output is in high-z
    if (freqcount_current_channel & 0x08)
    {
        // bit 8 set: enable multiplexer ic B
        PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_CS_A);
        PIN_CLEAR(FREQCOUNT_CHANNEL_MULTIPLEX_CS_B);
    }
    else
    {
        // bit 8 not set: enable multiplexer ic A
        PIN_CLEAR(FREQCOUNT_CHANNEL_MULTIPLEX_CS_A);
        PIN_SET(FREQCOUNT_CHANNEL_MULTIPLEX_CS_B);
    }
#endif

#if FREQCOUNT_CHANNELS > 16
#error sorry, more than 16 frequency counter channels currently not supported
#endif
}

void freqcount_set_state(uint8_t state,uint8_t channel)
{
    freqcount_perchannel_data[channel].enabled=state;

    if (!state)
    {
#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
        freqcount_perchannel_data[channel].moving_average_pointer=255;
#else // FREQCOUNT_MOVING_AVERAGE_SUPPORT
        freqcount_perchannel_data[channel].ticks_result_sum=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_perchannel_data[channel].duty_result_sum=0;
#endif
#endif // FREQCOUNT_MOVING_AVERAGE_SUPPORT
    }
}

/*
  -- Ethersex META --
  header(services/freqcount/freqcount.h)
  init(freqcount_init_measure)
*/
