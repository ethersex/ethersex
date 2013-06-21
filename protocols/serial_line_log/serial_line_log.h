/*
 *
 * Copyright (c) 2009, 2010 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _SLL_H
#define _SLL_H

#include "config.h"

struct serial_line_log_data
{
  uint8_t data[SERIAL_LINE_LOG_COUNT + 1];
  uint8_t len;
  uint8_t timeout;
};

extern struct serial_line_log_data sll_data_new;
extern struct serial_line_log_data sll_data;

void serial_line_log_init (void);
void serial_line_log_periodic (void);

#endif /* _SLL_H */
