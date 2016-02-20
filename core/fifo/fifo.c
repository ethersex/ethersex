/*
 *
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
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

#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "fifo.h"

bool
fifo_init(fifo_t * fifo, uint16_t fifo_size, uint8_t item_size)
{
  fifo->fifo = malloc(fifo_size * item_size);
  if (fifo->fifo == NULL)
    return false;
  fifo->fifo_end = (char *) fifo->fifo + fifo_size * item_size;
  fifo->fifo_size = fifo_size;
  fifo->count = 0;
  fifo->item_size = item_size;
  fifo->head = fifo->fifo;
  fifo->tail = fifo->fifo;

  return true;
}

void
fifo_free(fifo_t * fifo)
{
  free(fifo->fifo);
}

bool
fifo_put(fifo_t * fifo, const void *item)
{
  if (fifo->count == fifo->fifo_size)
    return false;
  memcpy(fifo->head, item, fifo->item_size);
  fifo->head = (char *) fifo->head + fifo->item_size;
  if (fifo->head == fifo->fifo_end)
    fifo->head = fifo->fifo;
  fifo->count++;
  return true;
}

bool
fifo_get(fifo_t * fifo, void *item)
{
  if (fifo->count == 0)
    return false;
  memcpy(item, fifo->tail, fifo->item_size);
  fifo->tail = (char *) fifo->tail + fifo->item_size;
  if (fifo->tail == fifo->fifo_end)
    fifo->tail = fifo->fifo;
  fifo->count--;

  return true;
}

bool
fifo_empty(fifo_t * fifo)
{
  if (fifo->count == 0)
    return true;
  return false;
}
