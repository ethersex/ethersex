/*
 *
 * Copyright (c) 2011 by Erik Kunze <ethersex@erik-kunze.de>
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
#ifndef HOST_AVR_PGMSPACE_H
#define HOST_AVR_PGMSPACE_H

#include <string.h>

int printf_P (const char *fmt, ...);
int sprintf_P (char *, const char *fmt, ...);
int snprintf_P (char *, int, const char *fmt, ...);

#define sscanf_P(a...)		sscanf(a)

#define memcmp_P(a...)		memcmp(a)
#define memcpy_P(a...)		memcpy(a)

#define strlen_P(a...)		strlen(a)
#define strstr_P(a...)		strstr(a)
#define strcat_P(a...)		strcat(a)
#define strcpy_P(a...)		strcpy(a)
#define strcmp_P(a...)		strcmp(a)
#define strncmp_P(a...)		strncmp(a)
#define strncasecmp_P(a...)	strncasecmp(a)

#define pgm_read_dword(a)	(*(a))
#define pgm_read_word(a)	(*(a))
#define pgm_read_byte(a)	(*(a))

#define PSTR(a)	(a)
#define PROGMEM

typedef const char * PGM_P;

#endif  /* HOST_AVR_PGMSPACE_H */
