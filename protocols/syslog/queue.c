/*
 *
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
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
  Node *help = malloc(sizeof(Node));

  if (help == NULL)
    return 0;

  help->prev = NULL;
  help->data = data;

  if (queue->end == NULL)       // queue is empty
    queue->end = help;
  else
    queue->start->prev = help;

  help->next = queue->start;
  queue->start = help;

  return 1;
}

char *
pop(Queue * queue)
{
  Node *help;
  char *data;
  help = queue->end;

  if (help != NULL)
  {
    queue->end = queue->end->prev;

    if (queue->end == NULL)     // queue is empty
      queue->start = NULL;
    else
      queue->end->next = NULL;

    help->prev = NULL;
    data = help->data;
    free(help);

    return data;
  }

  return NULL;
}



uint8_t
isEmpty(const Queue * queue)
{
  if (queue->end == NULL && queue->start == NULL)
    return 1;

  return 0;
}
