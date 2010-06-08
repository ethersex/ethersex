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

#ifndef _PWM_MELODY_H
#define _PWM_MELODY_H

struct notes_duration_t
{
  uint16_t note;
  uint16_t duration;
};

struct song_t
{
  const PGM_P title;
  const uint8_t delay;
  const uint8_t transpose;
  const struct notes_duration_t *notes;
  const uint16_t size;
};

void 
pwm_melody_init(uint8_t nr);

#endif /* _PWM_MELODY_H */

