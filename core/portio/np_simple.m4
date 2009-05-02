dnl
dnl   Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 3 as
dnl   published by the Free Software Foundation.
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
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl
divert(5)

/* This is just to make the inversion more visible. */
#define INVERT(a) (!(a))

#define NP_PORT(a) (PORT ## a)
#define NP_PIN(a) (PIN ## a)
#define NP_DDR(a) (DDR ## a)

const char PROGMEM np_str_on[] = "on";
const char PROGMEM np_str_off[] = "off";

static char
np_simple_check (char *cmd)
{
  /* Check for either `pin set ... 1' or `pin set ... on',
     for reasons of simplicity we simply check for the `1' or the
     `n' of on. */
  if (cmd[0] == '1' || cmd[1] == 'n')
    return 1;
  else if (cmd[0] == '0' || cmd[1] == 'f')
    return 0;

  return -1;
}

divert(-1)
define(`np_simple_implement_out', `dnl
divert(5)int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
{
  /* config: $2 $3 */
  char i = np_simple_check (cmd);
  if (i < 0) return -1;

  if ($3(i))
    NP_PORT(substr($2, 1, 1)) |= _BV($2);
  else
    NP_PORT(substr($2, 1, 1)) &= ~_BV($2);

  return snprintf_P(output, len, i ? np_str_on : np_str_off);
}
')

define(`np_simple_implement_in', `dnl
divert(5)int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
{
  /* config: $2 $3 */
  uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);
  return snprintf_P(output, len, $3(i) ? np_str_on : np_str_off);
}
')

define(`np_simple_implement_toggle', `dnl
divert(5)int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
{
  /* config: $2 $3 */

#ifdef PINx_TOGGLE_WORKAROUND
  /* We cannot toggle the pin by writing to the PINx register, therefore
     we have to do a read, xor, write with interrupts disabled to make
     sure noone interferes ...
     Furthermore we have to check whether the pin really is configured
     as output, to not accidentally toggle the internal pull-up resistor. */

  if ((NP_DDR(substr($2, 1, 1)) & _BV($2)) == 0)
    return -1;   		/* Configured as input, stop. */

  /* Disable interrupts to omit interference */
  uint8_t sreg = SREG; cli();

  uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);
  NP_PORT(substr($2, 1, 1)) = i ^ _BV($2);

  SREG = sreg;			/* Possibly re-enable interrupts. */
#else  /* PINx_TOGGLE_WORKAROUND */
  /* First we read the current pin-state and afterwards toggle
     the pin by writing to the PINx register. */
  uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);

  /* now toggle the port */
  NP_PIN(substr($2, 1, 1)) |= _BV($2);
#endif  /* not PINx_TOGGLE_WORKAROUND */

  /* say just the opposite of the old situation ... */
  return snprintf_P(output, len, $3(i) ? np_str_off : np_str_on);
}
')

define(`np_simple_normal_out', `
ecmd_feature(np_simple_out_$2, "pin set $2 ")
np_simple_implement_out(np_simple_out_$2, $1, `')
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `')
ecmd_feature(np_simple_toggle_$2, "pin toggle $2")
np_simple_implement_toggle(np_simple_toggle_$2, $1, `')
')

define(`np_simple_inverted_out', `
ecmd_feature(np_simple_out_$2, "pin set $2 ")
np_simple_implement_out(np_simple_out_$2, $1, `INVERT')
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `INVERT')
ecmd_feature(np_simple_toggle_$2, "pin toggle $2")
np_simple_implement_toggle(np_simple_toggle_$2, $1, `INVERT')
')

define(`np_simple_normal_in', `
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `')
')

define(`np_simple_inverted_in', `
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `INVERT')
')
