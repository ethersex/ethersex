/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>

#include "protocols/syslog/syslog.h"
#include "protocols/syslog/syslog_debug.h"

#define MAX_SYSLOG_DEBUG_BUFFER 100

static FILE syslog_stream =
FDEV_SETUP_STREAM(syslog_debug_put, NULL, _FDEV_SETUP_WRITE);
static char syslog_debug_buf[MAX_SYSLOG_DEBUG_BUFFER + 1];

int
syslog_debug_put(char d, FILE * stream)
{
  uint8_t offset = strlen(syslog_debug_buf);

  if (d != '\n')
  {
    syslog_debug_buf[offset++] = d;
    syslog_debug_buf[offset] = 0;
  }

  if (d == '\n' || offset >= MAX_SYSLOG_DEBUG_BUFFER)
  {
    syslog_send(syslog_debug_buf);
    syslog_debug_buf[0] = 0;
  }
  return 0;
}

void
syslog_debug_init(void)
{
  stdout = &syslog_stream;
  stderr = &syslog_stream;
}

/*
  -- Ethersex META --
  header(protocols/syslog/syslog_debug.h)
  init(syslog_debug_init)
*/
