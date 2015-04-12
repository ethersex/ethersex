/*
 * scheduler/dynamic.h
 *
 * Copyright (c) 2013-2015 by Michael Brakemeier <michael@brakemeier.de>
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

#ifndef DYNAMIC_H_
#define DYNAMIC_H_

#include <stdint.h>

#include "scheduler.h"

/**
 * Add a dynamic timer.
 */
int scheduler_add_timer(timer_t func, uint16_t interval);

/**
 * Delete a dynamic timer.
 *
 * Note: A timer *MUST NOT* delete itself!
 */
int scheduler_delete_timer(timer_t func);

#endif /* DYNAMIC_H_ */
