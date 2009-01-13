/* 
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
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

#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "../config.h"
#include "../debug.h"
#include "../i2c_master/i2c_master.h"
#include "../i2c_master/i2c_lm75.h"
#include "../i2c_master/i2c_24CXX.h"
#include "../i2c_master/i2c_pca9531.h"
#include "ecmd.h"

#ifdef I2C_DETECT_SUPPORT

int16_t
parse_cmd_i2c_detect(char *cmd, char *output, uint16_t len)
{
  /* First call, we initialize our magic bytes*/
  if (cmd[0] != 0x23) {
    cmd[0] = 0x23;
    cmd[1] = 0;
  }
  uint8_t next_address = i2c_master_detect(cmd[1], 127);
  cmd[1] = next_address + 1;

  if (next_address > 127) /* End of scaning */
    return 0;
  else
    return -10 - snprintf_P(output, len, PSTR("detected at: 0x%x (%d)"), next_address, next_address);
}

#endif  /* I2C_DETECT_SUPPORT */

#ifdef I2C_LM75_SUPPORT

int16_t
parse_cmd_i2c_lm75(char *cmd, char *output, uint16_t len)
{
  while(*cmd == ' ') cmd++;
  if (*cmd < '0' || *cmd > '7') return -1;
  int16_t temp = i2c_lm75_read_temp(0x48 + (cmd[0] - '0'));
  if (temp == 0xffff)
    return snprintf_P(output, len, PSTR("no sensor detected"));

  return snprintf_P(output, len, PSTR("temperature: %d.%d"), temp / 10, temp % 10);
}

#endif  /* I2C_LM75_SUPPORT */

#ifdef I2C_PCA9531_SUPPORT
int16_t
parse_cmd_i2c_pca9531(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t period;
  uint8_t duty;
  sscanf_P(cmd, PSTR("%u %x %x"), &adr, &period, &duty);
  
#ifdef DEBUG_I2C
  debug_printf("I2C PCA9531 IC %u: pwm period %X; pwm duty: %X\n",adr, period, duty);
#endif
  i2c_pca9531_set(0xC0>>1 + adr, period,duty,0x00,0x40,0xEF,0x55);

  return snprintf_P(output, len, PSTR("pwm set"));
}

#endif  /* I2C_PCA9531_SUPPORT */
