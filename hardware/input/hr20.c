/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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
#include "hr20.h"

static uint8_t stellrad_status;
struct hr20_input hr20_input;

void
hr20_input_init (void)
{
  PCMSK1 |= _BV (STELLRAD_A_PIN);
  PCMSK1 |= _BV (STELLRAD_B_PIN);
  PCMSK1 |= _BV (TASTE_AUTO_PIN);
  PCMSK1 |= _BV (TASTE_PROG_PIN);
  PCMSK1 |= _BV (TASTE_GRAD_PIN);
  EIMSK |= _BV (PCIE1);
  PIN_SET (TASTE_AUTO);
  PIN_SET (TASTE_PROG);
  PIN_SET (TASTE_GRAD);
}

ISR (PCINT1_vect)
{

  uint8_t pinb_tmp = PINB;
  uint8_t rad_b = (pinb_tmp & _BV (STELLRAD_B_PIN)) != 0;
  uint8_t rad_a = (pinb_tmp & _BV (STELLRAD_A_PIN)) != 0;
  char direction = rad_b == stellrad_status;

  if (rad_a == rad_b)
    {
      stellrad_status = rad_a;
    }
  else
    {
      uint16_t i = 0;
      do
	{
	  i++;
	  pinb_tmp = PINB;
	}
      while (i && (((pinb_tmp & _BV (STELLRAD_A_PIN)) != 0)
                   != ((pinb_tmp & _BV (STELLRAD_B_PIN)) != 0 )));
      stellrad_status = (pinb_tmp & _BV (STELLRAD_B_PIN)) != 0;
      if (i > 50)
	{
	  hr20_input.wheel_right = direction;
	  hr20_input.wheel_left = !direction;
	}
    }

  if (!PIN_HIGH (TASTE_AUTO))
    hr20_input.button_auto = 1;
  if (!PIN_HIGH (TASTE_PROG))
    hr20_input.button_prog = 1;
  if (!PIN_HIGH (TASTE_GRAD))
    hr20_input.button_temp = 1;
}

/*
  -- Ethersex META --
  header(hardware/input/hr20.h)
  init(hr20_input_init)
*/
