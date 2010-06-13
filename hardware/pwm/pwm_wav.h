/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#ifndef _PWM_WAV_H
#define _PWM_WAV_H

#include <avr/pgmspace.h>

#define WAVEBUFFERLEN 100

#define SOUNDFREQ 8000
#define SOUNDDIVISOR (F_CPU/64/SOUNDFREQ)

#ifndef VFS_PWM_WAV_SUPPORT
  // inline data
  PROGMEM extern char pwmsound[];
#endif /* VFS_PWM_WAV_SUPPORT */

void pwm_wav_init(void);
void pwm_stop(void);

#endif /* _PWM_WAV_H */
