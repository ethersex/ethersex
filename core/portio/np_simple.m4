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
divert(0)

/* This is just to make the inversion more visible. */
#define INVERT(a) (!(a))

#define NP_PORT(a) (PORT ## a)
#define NP_PIN(a) (PIN ## a)
#define NP_DDR(a) (DDR ## a)

#ifndef SOAP_SUPPORT
#define soap_rpc(a,b)	/* no soap */
#endif  /* SOAP_SUPPORT */

#ifdef ECMD_PARSER_SUPPORT
const char PROGMEM np_str_on[] = "on";
const char PROGMEM np_str_off[] = "off";

static int8_t
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

#define REPLY(output,state) do {                               \
  uint8_t plen = state ? 2 : 3;                                        \
  memcpy_P (output, state ? np_str_on : np_str_off, plen);     \
  return plen;                                                 \
} while(0)
#endif  /* ECMD_PARSER_SUPPORT */


divert(-1)
define(`np_simple_implement_out', `dnl
divert(0)dnl
#ifdef ECMD_PARSER_SUPPORT
int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
{
  /* config: $2 $3 */
  int8_t i = np_simple_check (cmd);
  if (i < 0) return -1;

  if ($3(i))
    NP_PORT(substr($2, 1, 1)) |= _BV($2);
  else
    NP_PORT(substr($2, 1, 1)) &= ~_BV($2);

  REPLY (output, i);
}
#endif  /* ECMD_PARSER_SUPPORT */
')


################################################################################
###                                                                          ###
###  ECMD Implementation Macros                                              ###
###                                                                          ###
################################################################################


define(`np_simple_implement_in', `dnl
divert(0)dnl
#ifdef ECMD_PARSER_SUPPORT
int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
{
  /* config: $2 $3 */
  uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);
  REPLY (output, $3(i));
}
#endif  /* ECMD_PARSER_SUPPORT */
')

define(`np_simple_implement_toggle', `dnl
divert(0)dnl
#ifdef ECMD_PARSER_SUPPORT
int16_t parse_cmd_$1 (char *cmd, char *output, uint16_t len)
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

  uint8_t i = NP_PORT(substr($2, 1, 1));
  NP_PORT(substr($2, 1, 1)) = i  ^ _BV($2);

  SREG = sreg;			/* Possibly re-enable interrupts. */
#else  /* PINx_TOGGLE_WORKAROUND */
  /* First we read the current pin-state and afterwards toggle
     the pin by writing to the PINx register. */
  uint8_t i = NP_PORT(substr($2, 1, 1));

  /* now toggle the port */
  NP_PIN(substr($2, 1, 1)) |= _BV($2);
#endif  /* not PINx_TOGGLE_WORKAROUND */

  /* say just the opposite of the old situation ... */
  REPLY (output, !$3(i & _BV($2)));
}
#endif  /* ECMD_PARSER_SUPPORT */
')


################################################################################
###                                                                          ###
###  SOAP Implementation Macros                                              ###
###                                                                          ###
################################################################################


define(`np_simple_implement_soap_out', `dnl
divert(0)dnl
#ifdef SOAP_SUPPORT
uint8_t $1 (uint8_t len, soap_data_t *args, soap_data_t *result)
{
  result->type = SOAP_TYPE_INT;

  /* config: $2 $3 */
  if (len == 0) {
    /* read */
    uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);
    result->u.d_int = $3(i) != 0;
  }
  else if (len == 1 && args[0].type == SOAP_TYPE_INT) {
    /* write */
    if ($3(args[0].u.d_int))
      NP_PORT(substr($2, 1, 1)) |= _BV($2);
    else
      NP_PORT(substr($2, 1, 1)) &= ~_BV($2);
    result->u.d_int = args[0].u.d_int != 0;
  }
  else
    return 1;		/* fuck off */

  return 0;
}
#endif  /* SOAP_SUPPORT */
')

define(`np_simple_implement_soap_in', `dnl
divert(0)dnl
#ifdef SOAP_SUPPORT
uint8_t $1 (uint8_t len, soap_data_t *args, soap_data_t *result)
{
  /* config: $2 $3 */
  if (len != 0) return 1;	/* we do not want args. */

  result->type = SOAP_TYPE_INT;

  uint8_t i = NP_PIN(substr($2, 1, 1)) & _BV($2);
  result->u.d_int = $3(i) != 0;
  return 0;
}
#endif  /* SOAP_SUPPORT */
')



define(`np_simple_normal_out', `
ecmd_feature(np_simple_out_$2, "pin set $2 ")
np_simple_implement_out(np_simple_out_$2, $1, `')
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `')
ecmd_feature(np_simple_toggle_$2, "pin toggle $2")
np_simple_implement_toggle(np_simple_toggle_$2, $1, `')

soap_rpc(soap_rpc_np_simple_out_$2, "$2")
np_simple_implement_soap_out(soap_rpc_np_simple_out_$2, $1, `')
')

define(`np_simple_inverted_out', `
ecmd_feature(np_simple_out_$2, "pin set $2 ")
np_simple_implement_out(np_simple_out_$2, $1, `INVERT')
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `INVERT')
ecmd_feature(np_simple_toggle_$2, "pin toggle $2")
np_simple_implement_toggle(np_simple_toggle_$2, $1, `INVERT')

soap_rpc(soap_rpc_np_simple_out_$2, "$2")
np_simple_implement_soap_out(soap_rpc_np_simple_out_$2, $1, `INVERT')
')

define(`np_simple_normal_in', `
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `')

soap_rpc(soap_rpc_np_simple_in_$2, "$2")
np_simple_implement_soap_in(soap_rpc_np_simple_in_$2, $1, `')
')

define(`np_simple_inverted_in', `
ecmd_feature(np_simple_in_$2, "pin get $2")
np_simple_implement_in(np_simple_in_$2, $1, `INVERT')

soap_rpc(soap_rpc_np_simple_in_$2, "$2")
np_simple_implement_soap_in(soap_rpc_np_simple_in_$2, $1, `INVERT')
')
