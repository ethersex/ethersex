/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef HAVE_CW_H
#define HAVE_CW_H


struct cw_sign_t {
    char character;
    uint8_t sign;
};

void
cw_periodic(void);

void
cw_boot(void);

int16_t
cw_send(char *cmd, char *output, uint16_t len);

void
send(char *str);

#include "config.h"
#ifdef DEBUG_CW
# include "core/debug.h"
# define CWDEBUG(a...)  debug_printf("CW: " a)
#else
# define CWDEBUG(a...)
#endif

#endif  /* HAVE_CW_H */

