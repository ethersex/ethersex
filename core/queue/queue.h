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


#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct Node Node;
struct Node
{
  Node *prev;
  Node *next;
  char *data;
};

typedef struct Queue Queue;
struct Queue
{
  Node *start;
  Node *end;
};

uint8_t push(char *data, Queue * queue);
char *pop(Queue * queue);
uint8_t isEmpty(const Queue * queue);


#endif /* _QUEUE_H */
