/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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
#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "config.h"
#include "cw.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef CW_PWM_FREQ_SUPPORT
#include "hardware/pwm/pwm_freq.h"
#endif /* CW_PWM_FREQ_SUPPORT */

#ifdef CW_RFM12_ASK_SUPPORT
#include "hardware/radio/rfm12/rfm12.h"
#include "hardware/radio/rfm12/rfm12_ask.h"
#endif /* CW_RFM12_ASK_SUPPORT */

#ifdef CW_BEACON_SUPPORT
void
cw_periodic(void)
{
  CWDEBUG ("beacon\n");
  send( ""CONF_CW_BEACON_TEXT);
}
#endif

#ifdef CW_BOOT_SUPPORT
void
cw_boot(void)
{
  CWDEBUG ("boot\n");
  send( ""CONF_CW_BOOT_TEXT);
}
#endif


static const char signs[][2] PROGMEM = {
 {' ', 0}, // do not modify this line
 {'a', 0b00000110},
 {'b', 0b00010001},
 {'c', 0b00010101},
 {'d', 0b00001001},
 {'e', 0b00000010},
 {'f', 0b00010100},
 {'g', 0b00001011},
 {'h', 0b00010000},
 {'i', 0b00000100},
 {'j', 0b00011110},
 {'k', 0b00001101},
 {'l', 0b00010010},
 {'m', 0b00000111},
 {'n', 0b00000101},
 {'o', 0b00001111},
 {'p', 0b00010110},
 {'q', 0b00011011},
 {'r', 0b00001010},
 {'s', 0b00001000},
 {'t', 0b00000011},
 {'u', 0b00001100},
 {'v', 0b00011000},
 {'w', 0b00001110},
 {'x', 0b00011001},
 {'y', 0b00011101},
 {'z', 0b00010011},
// {'ä', 0b00011010},
// {'ö', 0b00010111},
// {'ü', 0b00011100},
 {'0', 0b00111111},
 {'1', 0b00111110},
 {'2', 0b00111100},
 {'3', 0b00111000},
 {'4', 0b00110000},
 {'5', 0b00100000},
 {'6', 0b00100001},
 {'7', 0b00100011},
 {'8', 0b00100111},
 {'9', 0b00101111},
 {'"', 0b01010010},
 {'(', 0b00101101},
 {')', 0b01101101},
 {'+', 0b00101010},
 {',', 0b01110011},
 {'-', 0b01100001},
 {'.', 0b01101010},
 {'/', 0b00101001},
 {':', 0b01000111},
 {'=', 0b00110001},
 {'?', 0b01001100},
 {'@', 0b01011010} // see http://de.wikipedia.org/wiki/Morsecode#.40-Zeichen
};

#define CW_SIGNS sizeof(signs) / sizeof(struct cw_sign_t)
uint8_t convert(char c){
  struct cw_sign_t tab;
  uint8_t i;
  CWDEBUG ("convert %c, table %i\n", c, CW_SIGNS);
  for (i=0; i < CW_SIGNS; i++) {
	memcpy_P(&tab, &signs[i], sizeof(struct cw_sign_t));
    if (tab.character == c) {
      return tab.sign;
    }
    if (tab.character == (c | 0x60) ) {  // try uppercase
      return tab.sign;
    }
  }
  CWDEBUG ("unknown: '%c'\n",c);
  return 0;
}

#define DIT 1200/CW_WPM
#define DAH DIT*3
#define WORDBREAK DIT*7

void
send_sign(uint8_t c){
  int8_t i;
  CWDEBUG ("cw: %X: ", c);
  if (c == 0) {
    _delay_ms(WORDBREAK);
  } else {

#ifdef CW_RFM12_ASK_SUPPORT
  rfm12_prologue ();
#endif /* CW_RFM12_ASK_SUPPORT */

    for (i = 0; i < 8; i++) {
      if (c == 1) break;
#ifdef CW_PIN_SUPPORT
      PIN_SET(CW);
#endif /* CW_PIN_SUPPORT */
#ifdef CW_PWM_FREQ_SUPPORT
      pwm_freq_play(CW_PWM_FREQ);
#endif /* CW_PWM_FREQ_SUPPORT */
#ifdef CW_RFM12_ASK_SUPPORT
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
#endif /* CW_RFM12_ASK_SUPPORT */

      if ((c & 0x01) == 1 ) { // read lowest bit
  		CWDEBUG ("dah ");
        _delay_ms(DAH); // wait dah
      } else {
  		CWDEBUG ("dit ");
        _delay_ms(DIT); // wait dit
      }
#ifdef CW_PIN_SUPPORT
      PIN_CLEAR(CW);
#endif /* CW_PIN_SUPPORT */
#ifdef CW_PWM_FREQ_SUPPORT
      pwm_freq_play(0);
#endif /* CW_PWM_FREQ_SUPPORT */
#ifdef CW_RFM12_ASK_SUPPORT
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
#endif /* CW_RFM12_ASK_SUPPORT */
      _delay_ms(DIT); // wait dit during morse char
      c = c >> 1;  // right shift 
    }
#ifdef CW_RFM12_ASK_SUPPORT
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
#endif /* CW_RFM12_ASK_SUPPORT */
  }  
  CWDEBUG (".\n");
  _delay_ms(DAH); // break between morce chars
}

void
send(char *str){
  CWDEBUG ("send: %s\n", str);
  uint8_t i;
  uint8_t len = strlen(str);
  for (i = 0; i < len; i++) {
    send_sign(convert(str[i])); // convert char to sign

  }
}

int16_t parse_cmd_cw_send(char *cmd, char *output, uint16_t len) 
{
  send(cmd);
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block([[Morce_Code]])
  header(protocols/cw/cw.h)
  ecmd_feature(cw_send, "cw send ",MESSAGE, Send MESSAGE in Morce Code)
  ifdef(`conf_CW_BOOT', `init(cw_boot)')
  ifdef(`conf_CW_BEACON',`timer(200,cw_periodic())')
*/
