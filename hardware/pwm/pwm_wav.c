/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) by Ulrich Radig <mail@ulrichradig.de>
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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "pwm_wav.h"
#include "protocols/ecmd/ecmd-base.h"

/*
 HINT: the use of DEBUG is discurraged, as the printout will destroy
 the sound timing.
*/

#ifdef VFS_PWM_WAV_SUPPORT
  #include "core/vfs/vfs.h"
  uint8_t wavebuffer[WAVEBUFFERLEN];
  uint8_t wavebuffer_pos = WAVEBUFFERLEN;
  struct vfs_file_handle_t *handle=NULL;
  uint32_t filesize = 0;
#else
  #include "ethersex_wav.h"
  #define PWMSOUNDSIZE sizeof(pwmsound)
#endif /* VFS_PWM_WAV_SUPPORT */

uint16_t pwmbytecounter = 0;

//Timer2 Interrupt
ISR (TIMER0_OVF_vect)
{
#ifdef VFS_PWM_WAV_SUPPORT
    if (wavebuffer_pos == WAVEBUFFERLEN) {
        if (vfs_read(handle, wavebuffer, WAVEBUFFERLEN) <= 0 ){
			pwm_stop();
		}
        wavebuffer_pos = 0;
    }
    uint8_t s = wavebuffer[wavebuffer_pos++];
#else
	uint8_t s = pgm_read_byte(&pwmsound[pwmbytecounter]);
#endif /* VFS_PWM_WAV_SUPPORT */
#ifdef DEBUG_PWM
    	if (pwmbytecounter < 10 || ((pwmbytecounter % 1000) == 0) ) debug_printf("PWM sound %x at pos %u\n",s, pwmbytecounter);
#endif
	TC0_COUNTER_CURRENT = 255 - SOUNDDIVISOR;
	OCR2A = s;
	pwmbytecounter++;
#ifdef VFS_PWM_WAV_SUPPORT
	if(pwmbytecounter > filesize)
#else
	if(pwmbytecounter > PWMSOUNDSIZE)
#endif
	{
		pwm_stop();
	}
}

void
pwm_wav_init(void)
{
	pwmbytecounter = 0;
#ifdef DEBUG_PWM
    #ifdef VFS_PWM_WAV_SUPPORT
    	debug_printf("PWM vfs wav init, size: %lu, %u Hz\n", filesize, SOUNDFREQ );
    #else
    	debug_printf("PWM inline wav init, size: %u, %u Hz\n", PWMSOUNDSIZE, SOUNDFREQ );
    #endif /* VFS_PWM_WAV_SUPPORT */
#endif
	//Set TIMER2 (PWM OC2 Pin = PD7)
	DDRD |= (1<<7);
	TCCR2A |= (1<<WGM21|1<<WGM20|1<<COM2A1);
	TCCR2B |= (1<<CS20);
	OCR2A = 128;

	//Set TIMER0
	TIMSK0 |= (1 << TOIE0);
	TCCR0B = (1<<CS00|1<<CS01) ;
	TC0_COUNTER_CURRENT = 255 - SOUNDDIVISOR;
}

void
pwm_stop()
{
	pwmbytecounter = 0;
#ifdef VFS_PWM_WAV_SUPPORT
    wavebuffer_pos = WAVEBUFFERLEN;
    vfs_close(handle);
    filesize=0;
#endif /* VFS_PWM_WAV_SUPPORT */
#ifdef DEBUG_PWM
    	debug_printf("PWM stopped\n");
#endif
	// timer 2 stop
	TCCR2B = 0;

	// timer 0 stop
	TCCR0B = 0 ;

}

int16_t
parse_cmd_pwm_wav_play(char *cmd, char *output, uint16_t len)
{
#ifdef VFS_PWM_WAV_SUPPORT
	if (cmd[0] != '\0' ) {
    	handle = vfs_open(cmd+1);
		if (handle == NULL) {
#ifdef DEBUG_PWM
	    	debug_printf("file '%s' not found\n", cmd);
#endif
    		return ECMD_ERR_READ_ERROR;
		}
		filesize = vfs_size(handle);
	}
#endif /* VFS_PWM_WAV_SUPPORT */
    pwm_wav_init();
    return ECMD_FINAL_OK;
}

int16_t
parse_cmd_pwm_wav_stop(char *cmd, char *output, uint16_t len)
{
    pwm_stop();
    return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[Sound]]/WAV support)
  ecmd_feature(pwm_wav_play, "pwm wav", <FILENAME>,Play wave file. Use VFS if compiled in. More details at [[Sound]])
  ecmd_feature(pwm_wav_stop, "pwm stop", , Stop wav)
*/
