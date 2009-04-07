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
#include "config.h"
#include "core/debug.h"
#include "../i2c_master/i2c_master.h"
#include "../i2c_master/i2c_lm75.h"
#include "../i2c_master/i2c_24CXX.h"
#include "../i2c_master/i2c_pca9531.h"
#include "../i2c_master/i2c_pcf8574x.h"
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
  int16_t temp = i2c_lm75_read_temp(I2C_SLA_LM75 + (cmd[0] - '0'));
  if (temp == 0xffff)
    return snprintf_P(output, len, PSTR("no sensor detected"));

  return snprintf_P(output, len, PSTR("%d.%d"), temp / 10, temp % 10);
}

#endif  /* I2C_LM75_SUPPORT */

#ifdef I2C_PCA9531_SUPPORT
int16_t
parse_cmd_i2c_pca9531(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t period1;
  uint8_t duty1;
  uint8_t period2;
  uint8_t duty2;
  uint8_t firstnibble;
  uint8_t lastnibble;
  sscanf_P(cmd, PSTR("%u %x %x %x %x %x"), &adr, &period1, &duty1, &period2, &duty2, &firstnibble, &lastnibble);
  
#ifdef DEBUG_I2C
  debug_printf("I2C PCA9531 IC %u: pwm1 period %X, duty %X; pwm2 period %X, duty%X; %X %X\n",adr, period1, duty1, period2, duty2, firstnibble, lastnibble);
#endif
//  i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period, duty, 0x00, 0x40, 0xEF, 0x55);
  i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period1, duty1, period2, duty2, firstnibble, lastnibble);

  return snprintf_P(output, len, PSTR("pwm ok"));
}

#endif  /* I2C_PCA9531_SUPPORT */

#ifdef I2C_PCF8574X_SUPPORT
int16_t
parse_cmd_i2c_pcf8574x_read(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t chip;
  sscanf_P(cmd, PSTR("%u %u"), &adr, &chip);

  if (chip == 0) {
	  adr += I2C_SLA_PCF8574;
  }else{
	  adr += I2C_SLA_PCF8574A;
  }
#ifdef DEBUG_I2C
  debug_printf("I2C PCF8574X IC address 0x%X\n", adr);
#endif
  return snprintf_P(output, len, PSTR("%X"), i2c_pcf8574x_read(adr));
}

int16_t
parse_cmd_i2c_pcf8574x_set(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t chip;
  uint8_t value;
  sscanf_P(cmd, PSTR("%u %u %x"), &adr, &chip, &value);

  if (chip == 0) {
	  adr += I2C_SLA_PCF8574;
  }else{
	  adr += I2C_SLA_PCF8574A;
  }
#ifdef DEBUG_I2C
  debug_printf("I2C PCF8574X IC address 0x%X, value:%X\n",adr, value);
#endif
  i2c_pcf8574x_set(adr, value);

  return snprintf_P(output, len, PSTR("%X"), value);
}

#endif  /* I2C_PCF8574X_SUPPORT */
