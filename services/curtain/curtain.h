/*
 * Copyright (c) 2009 by David Gräff
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

#ifndef MOTORCURTAIN_H
#define MOTORCURTAIN_H

void motorCurtain_setPosition(uint8_t pos);
uint8_t motorCurtain_getPosition(void);
uint8_t motorCurtain_getMax(void);
void motorCurtain_off(void);

void broadcast_sensors(void);
void motorCurtain_init(void);
void motorCurtain_main(void);

#endif /* MOTORCURTAIN_H */
