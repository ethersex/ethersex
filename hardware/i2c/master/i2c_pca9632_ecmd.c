/*
 * Copyright (c) 2013 by Michael Schopferer <michael@schopferer.name>
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
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pca9632.h"
#include "protocols/ecmd/ecmd-base.h"


#ifdef I2C_PCA9632_SUPPORT
int16_t
parse_cmd_i2c_pca9632_reset(char *cmd, char *output, uint16_t len)
{

#ifdef DEBUG_I2C
        debug_printf("I2C PCA9632 Reset\n");
#endif
        i2c_pca9632_reset();
        return ECMD_FINAL_OK;
}

int16_t
parse_cmd_i2c_pca9632_set_reg(char *cmd, char *output, uint16_t len)
{
        uint8_t adr;
        uint8_t mode1;
        uint8_t mode2;
        uint8_t ledstate;
        sscanf_P(cmd, PSTR("%hhx %hhx %hhx %hhx"), 
                      &adr, &mode1, &mode2, &ledstate);

#ifdef DEBUG_I2C
        debug_printf("I2C PCA9632 Init\n");
#endif
        i2c_pca9632_init(adr, mode1, mode2, ledstate);
        return ECMD_FINAL_OK;
}

int16_t
parse_cmd_i2c_pca9632_set_blink(char *cmd, char *output, uint16_t len)
{
        uint8_t adr;
        uint8_t grppwm;
        uint8_t grpfreq;
        sscanf_P(cmd, PSTR("%hhx %hhx %hhx"), &adr, &grppwm, &grpfreq);

#ifdef DEBUG_I2C
        debug_printf("I2C PCA9632 set blink\n");
#endif
        i2c_pca9632_set_blink(adr, grppwm, grpfreq);
        return ECMD_FINAL_OK;
}

int16_t
parse_cmd_i2c_pca9632_set_led(char *cmd, char *output, uint16_t len)
{
        uint8_t adr;
        uint8_t led_x;
        uint8_t pwm_x;
        sscanf_P(cmd, PSTR("%hhx %hhx %hhx "), &adr, &led_x, &pwm_x);

#ifdef DEBUG_I2C
        debug_printf("I2C PCA9632 LED_x PWM set\n");
#endif
        i2c_pca9632_set_led(adr, led_x, pwm_x);
        return ECMD_FINAL_OK;
}

#endif  /* I2C_PCA9632_SUPPORT */

/*
  -- Ethersex META --
ecmd_ifdef(I2C_PCA9632_SUPPORT)
  ecmd_feature(i2c_pca9632_set_led, "pca9632 setled",CHIPADDR LED_x PWM_x,Set PWM of LED_x (LED0 - LED3))
  ecmd_feature(i2c_pca9632_set_reg, "pca9632 setreg",CHIPADDR MODE1 MODE2 LEDOUT,initialize PCA9632 with MODE1 MODE2 and LEDOUT)
  ecmd_feature(i2c_pca9632_set_blink, "pca9632 setgrp",CHIPADDR GRPPWM GRPFREQ,Set GRPPWM and GRPFREQ for blinking or dimming)
  ecmd_feature(i2c_pca9632_reset, "pca9632 reset",, reset PCA9632 to power-up values)
ecmd_endif
*/
