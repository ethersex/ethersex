/*
 *
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
 * and http://rn-wissen.de
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#ifndef _FIFO_H_
#define _FIFO_H_

#include "hardware/can/can.h"

typedef struct
{
	uint8_t volatile count;			// Zeichen im Puffer
	uint8_t size; 					// Puffer-Größe
	can_t *pread; 					// Lesezeiger
	can_t *pwrite; 					// Schreibzeiger
	uint8_t read2end, write2end; 	// Zeichen bis zum Überlauf Lese-/Schreibzeiger
} fifo_t;

extern void fifo_init(fifo_t*, can_t *buf, const uint8_t size);
extern uint8_t fifo_push(fifo_t*, const can_t *data);
extern uint8_t fifo_pull_wait(fifo_t*, can_t *data);
extern uint8_t fifo_pull_nowait(fifo_t*, can_t *data);
extern uint8_t fifo_check(fifo_t *fifo);

#endif // _FIFO_H_
