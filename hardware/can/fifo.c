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

#include "fifo.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

// Internal Function
uint8_t fifo_get(fifo_t *fifo, can_t *data);

/*---------------------------------------------------------------------
 FIFO init
 * --------------------------------------------------------------------*/
void fifo_init(fifo_t *fifo, can_t *buffer, const uint8_t size)
{
	fifo->count = 0;
	fifo->pread = fifo->pwrite = buffer;
	fifo->read2end = fifo->write2end = fifo->size = size;
}

/*---------------------------------------------------------------------
 Push Message to FIFO
 * --------------------------------------------------------------------*/
uint8_t fifo_push(fifo_t *fifo, const can_t *message)
{
	if (fifo->count >= fifo->size)
		return 0;

	can_t *pwrite = fifo->pwrite;

	*(pwrite++) = *message;

	uint8_t write2end = fifo->write2end;

	if (--write2end == 0)
	{
		write2end = fifo->size;
		pwrite -= write2end;
	}

	fifo->write2end = write2end;
	fifo->pwrite = pwrite;

	fifo->count++;

	return fifo->count;
}

/*---------------------------------------------------------------------
 Pull Message from FIFO - wait for Message
 * --------------------------------------------------------------------*/
uint8_t fifo_pull_wait(fifo_t *fifo, can_t *message)
{
	while (!fifo->count)
		;
	return fifo_get(fifo, message);
}

/*---------------------------------------------------------------------
 Pull Message from FIFO - no wait
 * --------------------------------------------------------------------*/
uint8_t fifo_pull_nowait(fifo_t *fifo, can_t *message)
{
	if (!fifo->count)
	{
		return 0;
	}
	return fifo_get(fifo, message);
}

/*---------------------------------------------------------------------
 Check for Messages in the FIFO
 * --------------------------------------------------------------------*/
uint8_t fifo_check(fifo_t *fifo)
{
	if (!fifo->count)
	{
		return 0;
	}
	return 1;
}

/*---------------------------------------------------------------------
 Get Message from FIFO
 * --------------------------------------------------------------------*/
uint8_t fifo_get(fifo_t *fifo, can_t *message)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (!fifo->count)
		{
			return 0;
		}

		can_t *pread = fifo->pread;
		*message = *(pread++);
		uint8_t read2end = fifo->read2end;

		if (--read2end == 0)
		{
			read2end = fifo->size;
			pread -= read2end;
		}

		fifo->pread = pread;
		fifo->read2end = read2end;

		fifo->count--;
	}
	return 1;
}
