/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef HAVE_PWM_DTMF_H
#define HAVE_PWM_DTMF_H

#define PWM_DTMF_SIGNAL			70
#define PWM_DTMF_SIGNAL_BREAK   30

//***************************  x_SW  ***************************************
//Table of x_SW (excess 8): x_SW = ROUND(8*N_samples*f*510/Fck)
//**************************************************************************

#define SAMPLES 		128
#define PRESCALER 		1
// 8*SAMPLES*f*510/(FREQ/PRESCALER)
#define recode(f...)  	(unsigned long)8*SAMPLES*f*512/(FREQ*PRESCALER)

//high frequency (coloum)
#define F1209 recode(1209)
#define F1336 recode(1336)
#define F1477 recode(1447)
#define F1633 recode(1633)
//low frequency (row)
#define F697  recode(697)
#define F770  recode(770)
#define F852  recode(852)
#define F941  recode(941)


struct dtmf_t
{
  char character;
  uint8_t high;
  uint8_t low;
};

int16_t 
parse_cmd_pwm_dtmf(char *cmd, char *output, uint16_t len);

int16_t 
parse_cmd_pwm_dtmfstr(char *cmd, char *output, uint16_t len);

void 
dtmf(char input);

void
pwm_dtmf_init(void);

#endif  /* HAVE_PWM_DTMF_H */
