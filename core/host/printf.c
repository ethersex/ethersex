/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <avr/pgmspace.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <glib.h>

static char *
printffmtfix (const char *a)
{
  char *ptr;

  a = g_strdup (a);
  while ((ptr = strstr (a, "%S")))
    ptr[1] = 's';

  return a;
}

int
printf_P (const char *fmt, ...)
{
  char *f = printffmtfix(fmt);

  va_list va;
  va_start (va, fmt);
  int r = vprintf (f, va);
  va_end (va);

  g_free (f);
  return r;
}

int
sprintf_P (char *s, const char *fmt,...)
{
  char *f = printffmtfix(fmt);

  va_list va;
  va_start (va, fmt);
  int r = vsprintf (s, f, va);
  va_end (va);

  g_free (f);
  return r;
}

int
snprintf_P (char *s, int n, const char *fmt,...)
{
  char *f = printffmtfix(fmt);

  va_list va;
  va_start (va, fmt);
  int r = vsnprintf (s, n, f, va);
  va_end (va);

  g_free (f);
  return r;
}
