/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "core/bit-macros.h"
#include "core/portio/named_pin.h"
#include "core/portio/portio.h"
#include "protocols/ecmd/ecmd-base.h"

static const char str_on[] PROGMEM = "on";
static const char str_off[] PROGMEM = "off";
static const char str_input[] PROGMEM = "error: pin is input";

/**
 * Get all named pins in a list (separator is the newline character).
 * Warning: this funtion return only that much entries that fit into
 * the output buffer.
 */
int16_t
parse_cmd_pin_list (char *cmd, char *output, uint16_t len)
{
  uint16_t retLen = 0;
  PGM_P text;

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
    {					/* indicator flag: real invocation:  0 */
      cmd[0] = ECMD_STATE_MAGIC;	/*                 continuing call: 23 */
      cmd[1] = 0;			/* counter for output lines */
    }

  while (1)
    {
      /* get named-pin from array */
      text = (const char *) pgm_read_word (&portio_pincfg[(uint8_t)
							  cmd[1]++].name);
      /* leave loop if end of array is reached */
      if (text == NULL)
	break;
      uint8_t lineLength = strlen_P (text);
      /* leave loop if output buffer is too small */
      if (retLen + lineLength + 1 > len)
	{
	  // if we get called again, we have to get this entry again, too.
	  (uint8_t) cmd[1]--;
	  break;
	}
      memcpy_P (output, text, lineLength);
      output += lineLength;
      /* add newline character */
      *output = '\n';
      ++output;
      retLen += lineLength + 1;
    }

  /* Remove last newline character if end of array is reached */
  --retLen;
  if (text == NULL)
    {
      return ECMD_FINAL (retLen);
    }
  else
    {
      output[retLen] = ECMD_NO_NEWLINE;
      return ECMD_AGAIN (retLen);
    }
}

int16_t
parse_cmd_pin_get (char *cmd, char *output, uint16_t len)
{
  uint8_t port, pin;

  /* Parse String */
  uint8_t retLen = sscanf_P (cmd, PSTR ("%hhu %hhu"), &port, &pin);
  /* Fallback to named pins */
  if (retLen != 2 && *cmd)
    {
      uint8_t pincfg = named_pin_by_name (cmd + 1);
      if (pincfg != 255)
	{
	  port = pgm_read_byte (&portio_pincfg[pincfg].port);
	  pin = pgm_read_byte (&portio_pincfg[pincfg].pin);
	  retLen = 2;
	}
    }
  if (retLen == 2 && port < IO_PORTS && pin < 8)
    {
      uint8_t pincfg = named_pin_by_pin (port, pin);
      uint8_t active_high = 1;
      if (pincfg != 255)
	active_high = pgm_read_byte (&portio_pincfg[pincfg].active_high);
      uint8_t val =
	XOR_LOG (vport[port].read_pin (port) & _BV (pin), !(active_high));
      return ECMD_FINAL (snprintf_P (output, len, val ? str_on : str_off));
    }

  return ECMD_ERR_PARSE_ERROR;
}


int16_t
parse_cmd_pin_set (char *cmd, char *output, uint16_t len)
{
  uint8_t port, pin, on;

  /* Parse String */
  uint8_t retLen = sscanf_P (cmd, PSTR ("%hhu %hhu %hhu"), &port, &pin, &on);
  /* Fallback to named pins */
  if (retLen != 3 && *cmd)
    {
      char *ptr = strchr (cmd + 1, ' ');
      if (ptr)
	{
	  *ptr = 0;
	  uint8_t pincfg = named_pin_by_name (cmd + 1);
	  if (pincfg != 255)
	    {
	      port = pgm_read_byte (&portio_pincfg[pincfg].port);
	      pin = pgm_read_byte (&portio_pincfg[pincfg].pin);
	      if (ptr[1])
		{
		  ptr++;
		  if (sscanf_P (ptr, PSTR ("%hhu"), &on) == 1)
		    retLen = 3;
		  else
		    {
		      if (strcmp_P (ptr, str_on) == 0)
			{
			  on = 1;
			  retLen = 3;
			}
		      else if (strcmp_P (ptr, str_off) == 0)
			{
			  on = 0;
			  retLen = 3;
			}
		    }
		}
	    }
	}
    }

  if (retLen == 3 && port < IO_PORTS && pin < 8)
    {
      PGM_P strOut = str_input;
      /* Set only if it is output */
      if (vport[port].read_ddr (port) & _BV (pin))
	{
	  uint8_t pincfg = named_pin_by_pin (port, pin);
	  uint8_t active_high = 1;
	  if (pincfg != 255)
	    active_high = pgm_read_byte (&portio_pincfg[pincfg].active_high);
	  uint8_t val = vport[port].read_port (port);
	  if (XOR_LOG (on, !active_high))
	    val |= _BV (pin);
	  else
	    val &= ~_BV (pin);
	  vport[port].write_port (port, val);
	  strOut = on ? str_on : str_off;
	}
      return ECMD_FINAL (snprintf_P (output, len, strOut));
    }

  return ECMD_ERR_PARSE_ERROR;
}

/* */

int16_t
parse_cmd_pin_toggle (char *cmd, char *output, uint16_t len)
{
  uint8_t port, pin;

  /* Parse String */
  uint8_t retLen = sscanf_P (cmd, PSTR ("%hhu %hhu"), &port, &pin);
  /* Fallback to named pins */
  if (retLen != 2 && *cmd)
    {
      uint8_t pincfg = named_pin_by_name (cmd + 1);
      if (pincfg != 255)
	{
	  port = pgm_read_byte (&portio_pincfg[pincfg].port);
	  pin = pgm_read_byte (&portio_pincfg[pincfg].pin);
	  retLen = 2;
	}
    }
  if (retLen == 2 && port < IO_PORTS && pin < 8)
    {
      PGM_P strOut = str_input;
      /* Toggle only if it is output */
      if (vport[port].read_ddr (port) & _BV (pin))
	{
	  uint8_t pincfg = named_pin_by_pin (port, pin);
	  uint8_t active_high = 1;
	  if (pincfg != 255)
	    active_high = pgm_read_byte (&portio_pincfg[pincfg].active_high);
	  uint8_t val = vport[port].read_port (port);
	  val ^= _BV (pin);
	  vport[port].write_port (port, val);
	  strOut = XOR_LOG (val & _BV (pin), !active_high) ? str_on : str_off;
	}
      return ECMD_FINAL (snprintf_P (output, len, strOut));
    }

  return ECMD_ERR_PARSE_ERROR;
}


/*
  -- Ethersex META --
  block([[Named_PIN]])
  ecmd_feature(pin_list, "pin list",, List all known named-pins.)
  ecmd_feature(pin_get, "pin get", NAME, Read and display the status of pin NAME.)
  ecmd_feature(pin_set, "pin set", NAME STATUS, Set the status of pin NAME to STATUS.)
  ecmd_feature(pin_toggle, "pin toggle", NAME, Toggle the status of pin NAME.)
*/
