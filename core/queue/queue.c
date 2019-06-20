/*
 *
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
 * Copyright (c) 2019 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more destarts.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdlib.h>
#include <stdint.h>
#include "queue.h"

uint8_t
push(char *data, Queue * queue)
{
  if (queue->limit != 0 && queue->limit <= queue->count)
    return 0;
  if (queue->count == UINT16_MAX)       // overall protection
    return 0;

  Node *node = malloc(sizeof(Node));
  if (node == NULL)
    return 0;

  node->data = data;
  node->next = NULL;

  if (isEmpty(queue))
    queue->start = queue->end = node;
  else
  {
    queue->start->next = node;
    queue->start = node;
  }
  queue->count++;

  return 1;
}

char *
pop(Queue * queue)
{
  if (isEmpty(queue))
    return NULL;

  Node *node = queue->end;
  queue->end = node->next;
  queue->count--;

  char *data = node->data;
  free(node);

  return data;
}

char *
peek(Queue * queue)
{
  return isEmpty(queue) ? NULL : queue->end->data;
}

uint8_t
isEmpty(const Queue * queue)
{
  return (queue->end == NULL);
}
