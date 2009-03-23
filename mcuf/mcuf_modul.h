/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */

#ifndef _MCUF_MODUL_H
#define _MCUF_MODUL_H

#include <stdint.h>
#include "../config.h"
#include "mcuf.h"

#ifdef MCUF_MODUL_SUPPORT
void mcuf_chess(void);
void mcuf_clean(uint8_t delay);
void mcuf_spiral(uint8_t delay);

#endif  /* MCUF_MODUL_SUPPORT */


#endif  /* _MCUF_MODUL_H */
