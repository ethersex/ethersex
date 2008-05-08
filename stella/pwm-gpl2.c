/*
 Copyright(C) 2006 Christian Dietrich <stettberger@dokucode.de>
 Copyright(C) 2006 Jochen Roessner <jochen@lugrot.de>
 Copyright(C) 2008 Jochen Roessner <jochen@lugrot.de>
 Copyright(C) 2007 Stefan Siegl <stesie@brokenpipe.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <inttypes.h>

#include "../config.h"
#include "stella.h"

volatile uint8_t timetable[STELLA_PINS][2];
volatile uint8_t i_timetable[STELLA_PINS][2];
volatile uint8_t length;
volatile uint8_t i_length;
volatile uint8_t now = 0;
volatile uint8_t overflow_mask = 0;
volatile uint8_t i_overflow_mask = 0;
volatile uint8_t update_table = 0;

void
stella_pwm_init(void)
{
  /* Normal PWM Mode */
  /* 256 Prescaler */
  _TCCR2_PRESCALE |= _BV(CS21);
  _TCCR2_PRESCALE |= _BV(CS22);

  /* Int. bei Overflow und CompareMatch einschalten */
  _TIMSK_TIMER2 |= _BV(TOIE2) | _BV(_OUTPUT_COMPARE_IE2); 

  STELLA_DDR = ((1 << STELLA_PINS) - 1) << STELLA_OFFSET;
}

SIGNAL(_SIG_OUTPUT_COMPARE2)
{
  STELLA_PORT &= ~i_timetable[now][1];

  now ++;
  now = now % i_length;

  _OUTPUT_COMPARE_REG2 = i_timetable[now][0];
}

SIGNAL(_SIG_OVERFLOW2)
{
  if(update_table == 1)
    {
      uint8_t i;
      for (i = 0; i < length; i ++)
	{
	  i_timetable[i][0] = timetable[i][0];
	  i_timetable[i][1] = timetable[i][1];
	}

      i_length = length;
      i_overflow_mask = overflow_mask;
      now = 0;
      update_table = 0;
    }

  STELLA_PORT |= i_overflow_mask;
}


void
stella_sort(uint8_t color[])
{
  uint8_t i;
  uint8_t y;
  uint8_t x = 0;
  uint8_t temp[STELLA_PINS][2];

  /* Schauen ob schon vorhanden */
  for (i = 0; i < STELLA_PINS; i ++)
    {
      temp[i][0] = 0;
      temp[i][1] = 0;
      uint8_t vorhanden = 0;
      for (y = 0; y < x; y ++)
	{
	  if (color[i] == temp[y][0])
	    vorhanden=1;
	}

      if (! vorhanden)
	{
	  temp[x][0] = color[i];
	  x ++;
	}
    }

  /* Sotieren */
  if (x != 1)
    {
      uint8_t t;
      for (y = 0; y < (x - 1); y ++)
	{
	  for (i = 0; i < (x - 1) - y; i ++)
	    {
	      if (temp[i][0] > temp[i + 1][0])
		{
		  t = temp[i][0];
		  temp[i][0] = temp[i + 1][0];
		  temp[i + 1][0] = t;
		}
	    }
	}
    }

  /* Eintragen */
  for (i = 0; i < STELLA_PINS; i ++)
    {
      for(y = 0; y < x; y ++)
	{
	  if (color[i] == temp[y][0])
	    {
	      temp[y][1] |= (1 << (i + STELLA_OFFSET));
	    }
	}
    }

  /* So etz noch die alten Werte ersetzen */
  while (update_table);

  for (i = 0; i < x; i ++)
    {
      timetable[i][0] = temp[i][0];
      timetable[i][1] = temp[i][1];
    }

  length = x;

  /* Overflow_mask neu bilden */
  overflow_mask = ((1 << STELLA_PINS) - 1) << STELLA_OFFSET;

  if (timetable[0][0] == 0)
    overflow_mask &= ~timetable[0][1];

  update_table = 1;
}

