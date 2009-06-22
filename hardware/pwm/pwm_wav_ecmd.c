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

#include "config.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef PWM_WAV_SUPPORT

#include "pwm_wav.h"

int16_t
parse_cmd_pwm_wav_play(char *cmd, char *output, uint16_t len)
{
    pwm_wav_init();
    return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM wav play")));
}

int16_t
parse_cmd_pwm_wav_stop(char *cmd, char *output, uint16_t len)
{
    pwm_stop();
    return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM wav stop")));
}

#endif  /* PWM_SUPPORT */
