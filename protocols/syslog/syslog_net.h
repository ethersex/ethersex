/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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


#ifndef _SYSLOG_NET_H
#define _SYSLOG_NET_H

/* constants */
#define SYSLOG_PORT 514
#define SYSLOG_CALLBACKS 3

/* This callback is called when the syslog connection is able to send data to
 * the syslog server
 */
typedef void (*syslog_callback_t)(void *data);

struct SyslogCallbackCtx {
  syslog_callback_t callback;
  void *data;
};

void syslog_net_init(void);
void syslog_net_main(void);
uint8_t syslog_insert_callback(syslog_callback_t callback, void *data);

#endif /* _SYSLOG_NET_H */
