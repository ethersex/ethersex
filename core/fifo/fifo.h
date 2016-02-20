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

#ifndef FIFO_H_
#define FIFO_H_

#include <stdbool.h>

typedef struct
{
  void *fifo;
  void *fifo_end;
  uint16_t fifo_size;
  uint8_t count;
  uint8_t item_size;
  void *head;
  void *tail;
} fifo_t;

bool fifo_init(fifo_t * fifo, uint16_t fifo_size, uint8_t item_size);

void fifo_free(fifo_t * fifo);

bool fifo_put(fifo_t * fifo, const void *item);

bool fifo_get(fifo_t * fifo, void *item);

bool fifo_empty(fifo_t * fifo);

#endif // FIFO_H_
