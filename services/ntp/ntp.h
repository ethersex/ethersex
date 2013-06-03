/*
 *
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef NTP_NTP_H
#define NTP_NTP_H

#include "protocols/uip/uip.h"

#define JAN_1970 2208988800

struct ntp_date_time {
  uint32_t seconds;
  uint32_t fraction;
};

struct ntp_packet {
  uint8_t  li_vn_mode;                    /* leap indicator, version and mode */
  uint8_t  stratum;                       /* peer stratum */
  uint8_t  ppoll;                         /* peer poll interval */
  signed char precision;                  /* peer clock precision */
  uint32_t rootdelay;                     /* distance to primary clock */
  uint32_t rootdispersion;                /* clock dispersion */
  uint32_t refid;                         /* reference clock ID */
  struct ntp_date_time    reftime;        /* time peer clock was last updated */
  struct ntp_date_time    org;            /* originate time stamp */
  struct ntp_date_time    rec;            /* receive time stamp */
  struct ntp_date_time    xmt;            /* transmit time stamp */
};

void ntp_init(void);
void ntp_conf(uip_ipaddr_t *ntpserver);
void ntp_newdata(void);
void ntp_periodic(void);
void ntp_send_packet(void);
void ntp_dns_query_cb(char *name, uip_ipaddr_t *ipaddr);
uip_ipaddr_t *ntp_getserver(void);

#endif /* NTP_NTP_H */
