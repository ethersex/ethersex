/*
 * Copyright (c) 2009 by Stefan Müller <mueller@cos-gbr.de>
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

#ifndef HAVE_SMS77_H
#define HAVE_SMS77_H

#define SMS77_VALUESIZE 16

char sms77_user[SMS77_VALUESIZE];
char sms77_pass[SMS77_VALUESIZE];
char sms77_recv[SMS77_VALUESIZE];
char sms77_type[SMS77_VALUESIZE];


uint8_t sms77_send(char *);
void sms77_init(void);


#include "config.h"
#ifdef DEBUG_SMS77
# include "core/debug.h"
# define SMSDEBUG(a...)  debug_printf("sms77: " a)
#else
# define SMSDEBUG(a...)
#endif

#endif  /* HAVE_SMS77_H */
