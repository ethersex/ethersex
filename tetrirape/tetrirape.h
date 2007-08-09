/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _TETRIRAPE_H
#define _TETRIRAPE_H

#include <stdint.h>

/* configuration constants */
#define CONFIG_INITIAL_LEVEL 1
#define CONFIG_LINES_PER_LEVEL 2
#define CONFIG_LEVEL_INC 1
#define CONFIG_SPECIAL_LINES 1
#define CONFIG_SPECIAL_COUNT 1
#define CONFIG_SPECIAL_CAPACITY 18
#define CONFIG_PIECES {14, 14, 15, 14, 14, 14, 15}
#define CONFIG_SPECIALS {18, 18, 3, 12, 0, 16, 3, 12, 18}
#define CONFIG_LEVEL_AVERAGE 1
#define CONFIG_CLASSIC_MODE 1

/* prototypes */
void tetrirape_try_unqueue(int mode);
void tetrirape_eat_data(void);
void tetrirape_disconnect(void);

#endif
