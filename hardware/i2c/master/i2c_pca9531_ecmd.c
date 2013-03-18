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

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pca9531.h"
#include "protocols/ecmd/ecmd-base.h"


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
        sscanf_P(cmd, PSTR("%hhu %hhx %hhx %hhx %hhx %hhx %hhx"), &adr, &period1, &duty1, &period2, &duty2, &firstnibble, &lastnibble);

#ifdef DEBUG_I2C
        debug_printf("I2C PCA9531 IC %u: pwm1 period %X, duty %X; pwm2 period %X, duty%X; %X %X\n",adr, period1, duty1, period2, duty2, firstnibble, lastnibble);
#endif
        //  i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period, duty, 0x00, 0x40, 0xEF, 0x55);
        i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period1, duty1, period2, duty2, firstnibble, lastnibble);

        return ECMD_FINAL(snprintf_P(output, len, PSTR("pwm ok")));
}

#endif  /* I2C_PCA9531_SUPPORT */

/*
  -- Ethersex META --
ecmd_ifdef(I2C_PCA9531_SUPPORT)
  ecmd_feature(i2c_pca9531, "pca9531",ADDR PERIODPWM1 DUTYPWM1 PERIODPWM2 DUTYPWM2 LED0..3 LED4..7, set PWM1 and PWM2 and LED states)
ecmd_endif
*/
