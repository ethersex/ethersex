/*
* frequency counter
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

#ifndef FREQCOUNT_NOSLOW_SUPPORT
// increased every 65536 clock cycles
// is a 24 bit clock together with TCNT1
volatile uint8_t timer_overflows;
#endif

// counts the number of full timer overflows
// since freqcount_start was set
// used to detect a frequency lower than we can measure
volatile uint8_t overflows_since_freq_start=0;

// full overflows needed to next clock tick
volatile uint8_t overflows_to_clocktick;

#define FULL_OVERFLOWS ((FREQCOUNT_CLOCKFREQ/MAX_OVERFLOW/HZ)+1)
#define REMAINING_TICKS ((uint16_t)((FREQCOUNT_CLOCKFREQ%(MAX_OVERFLOW*HZ))/HZ))

#if (FULL_OVERFLOWS < 2)
#error cpu frequency too low for frequency counter 
#endif

void freqcount_init (void)
{
    TCNT1 = 0;
    OCR1A = REMAINING_TICKS;
    overflows_to_clocktick=FULL_OVERFLOWS;

#ifndef FREQCOUNT_NOSLOW_SUPPORT
    timer_overflows=0;
#endif
    
    // make sure FREQCOUNT_PIN is defined and an input
    DDR_CONFIG_IN(FREQCOUNT_PIN);
}

// timer overflow
inline void timer_overflow()
{
#ifndef FREQCOUNT_NOSLOW_SUPPORT
    timer_overflows++;
    
    // did we have an overflow of timer_overflows?
    if (timer_overflows==0)
#endif   // this endif is in between if and { intentionally, no bug!
    {
        // we had an overflow
        // either of timer_overflows or of TCNT1
        overflows_since_freq_start++;
    }
        
    overflows_to_clocktick--;
}

ISR (TIMER1_OVF_vect)
{
    timer_overflow();
}

// timer compare
ISR(TIMER1_COMPA_vect)
{
    // did the overflow and compare happen at once?
    // make sure that the overflow vector is executed first
    // ignore cases where compare triggered first but could
    // not be handled yet. Limit at half the possible range for this.
    if (TIFR1 & _BV(TOV1) && OCR1A < 32768)
    {
        timer_overflow();
        // disable the int flag as we already have handled it
        TIFR1 = _BV(TOV1);
    }
    
    // we only have to look for incomplete timer overflow cycles
    if (overflows_to_clocktick==0)
    {
        overflows_to_clocktick=FULL_OVERFLOWS;
        OCR1A=REMAINING_TICKS-MAX_OVERFLOW+OCR1A;
        if (OCR1A > REMAINING_TICKS)
            overflows_to_clocktick--;

        // call the regular ISR for timer expired condition (every 20ms)
        timer_expired();
    }
}

// variables to communicate between pin change ISR and control
volatile tick24bit_t freqcount_start;
volatile tick24bit_t freqcount_freq_end;
#ifdef FREQCOUNT_DUTY_SUPPORT
volatile tick24bit_t freqcount_on_end;
#endif
volatile freqcount_state_t freqcount_state = FC_DISABLED;

void freqcount_mainloop(void)
{
    if (freqcount_state==FC_DISABLED)
        start_measure();
    else if (freqcount_state==FC_DONE)
    {
        measure_done();
        // for now: automatically restart
        start_measure();
    }
    
    check_measure_timeout();
}

static void start_measure(void)
{
    freqcount_state=FC_BEFORE_START;
    
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

static void measure_done(void)
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

    average_results(freqcount_ticks, freqcount_duty);
#else
    average_results(freqcount_ticks);
#endif
}

static void check_measure_timeout(void)
{
    static freqcount_state_t last_reset_state=FC_DISABLED;

    if (freqcount_state != last_reset_state)
    {
        overflows_since_freq_start=0;
        last_reset_state=freqcount_state;
    }
    else if (overflows_since_freq_start > 1 && 
        (freqcount_state==FC_BEFORE_START ||
         freqcount_state==FC_FREQ ||
         freqcount_state==FC_ON_CYCLE))
    {
        // we are inside a measurement, but we had two overflows
        // -> we can't get reliable data anymore

        // disable timer input capture interrupt
        TIMSK1 &= ~(_BV(ICIE1));

        freqcount_state=FC_DISABLED;
        last_reset_state=FC_DISABLED;
        overflows_since_freq_start=0;
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        average_results(0,0);
#else
        average_results(0);
#endif
    }
}

ISR(TIMER1_CAPT_vect)
{
    if (freqcount_state==FC_BEFORE_START)
    {
        // start to measure frequency
        freqcount_start.low=ICR1;
#ifndef FREQCOUNT_NOSLOW_SUPPORT
        freqcount_start.high=timer_overflows;
#endif
        freqcount_state=FC_FREQ;
    }
    else if (freqcount_state==FC_FREQ)
    {
        // next rising edge -> we have the frequency
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
    else if (freqcount_state==FC_ON_CYCLE)
    {
        freqcount_on_end.low=ICR1;
#ifndef FREQCOUNT_NOSLOW_SUPPORT
        freqcount_on_end.high=timer_overflows;
#endif /* FREQCOUNT_NOSLOW_SUPPORT */
        freqcount_state=FC_DONE;
#endif /* FREQCOUNT_DUTY_SUPPORT */
    }
}

uint32_t freqcount_ticks_result=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
uint8_t freqcount_duty_result=0;
#endif

#ifdef FREQCOUNT_DUTY_SUPPORT
static void average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty)
#else
static void average_results(uint32_t freqcount_ticks)
#endif
{
    static uint32_t freqcount_ticks_avgsum=0;
    static uint32_t freqcount_ticks_avg_max=0;
    static uint32_t freqcount_ticks_avg_min=0xFFFFFFFF;
    
#ifdef FREQCOUNT_DUTY_SUPPORT
    static uint16_t freqcount_duty_avgsum=0;
    static uint16_t freqcount_duty_avg_max=0;
    static uint16_t freqcount_duty_avg_min=0xFFFF;
#endif
    
    static uint8_t freqcount_avg_cnt=0;

    // collect FREQCOUNT_AVERAGE+2 values: min and max are cut off
    if (freqcount_avg_cnt < FREQCOUNT_AVERAGE+2 && freqcount_ticks != 0)
    {
        freqcount_ticks_avgsum+=freqcount_ticks;
        if (freqcount_ticks > freqcount_ticks_avg_max)
            freqcount_ticks_avg_max=freqcount_ticks;
        else if (freqcount_ticks < freqcount_ticks_avg_min)
            freqcount_ticks_avg_min=freqcount_ticks;

#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_duty_avgsum+=freqcount_duty;
        if (freqcount_duty > freqcount_duty_avg_max)
            freqcount_duty_avg_max=freqcount_duty;
        else if (freqcount_duty < freqcount_duty_avg_min)
            freqcount_duty_avg_min=freqcount_duty;
#endif

        freqcount_avg_cnt++;
    }
    else
    {
        if (freqcount_ticks == 0)
        {
            // we had a timeout, indicate this by setting the results to 0
            freqcount_ticks_result=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
            freqcount_duty_result=0;
#endif
        }
        else
        {
            freqcount_ticks_avgsum-=freqcount_ticks_avg_max;
            freqcount_ticks_avgsum-=freqcount_ticks_avg_min;
            freqcount_ticks_result=freqcount_ticks_avgsum/FREQCOUNT_AVERAGE;
            
#ifdef FREQCOUNT_DUTY_SUPPORT
            freqcount_duty_avgsum-=freqcount_duty_avg_max;
            freqcount_duty_avgsum-=freqcount_duty_avg_min;
            freqcount_duty_result=freqcount_duty_avgsum/FREQCOUNT_AVERAGE;
#endif
        }

        // reset averaging
        freqcount_ticks_avgsum=0;
        freqcount_ticks_avg_max=0;
        freqcount_ticks_avg_min=0xFFFFFFFF;
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_duty_avgsum=0;
        freqcount_duty_avg_max=0;
        freqcount_duty_avg_min=0xFFFF;
#endif
        
        freqcount_avg_cnt=0;

#ifdef FREQCOUNT_DEBUGGING
#ifdef FREQCOUNT_DUTY_SUPPORT
        debug_printf("fc ticks %lu, %lu Hz %u duty\n", freqcount_ticks_result,
                freqcount_get_freq_hz(),freqcount_duty_result);
#else
        debug_printf("fc ticks %lu, %lu Hz\n", freqcount_ticks_result,freqcount_get_freq_hz());
#endif
#endif
    }
}

uint32_t freqcount_get_freq_hz(void)
{
    uint32_t hz;
    
    if (freqcount_ticks_result==0)
        hz=0;
    else
    {
        // numerical correct rounding
        hz=(FREQCOUNT_CLOCKFREQ*10)/freqcount_ticks_result;
        hz+=5;
        hz/=10;
    }

    return hz;
}

/*
  -- Ethersex META --
  header(services/freqcount/freqcount.h)
*/
