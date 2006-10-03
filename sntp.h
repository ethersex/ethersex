/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
 }}} */

#ifndef _SNTP_H
#define _SNTP_H

#include <stdint.h>
#include "common.h"
#include "uip/uip.h"

/* see rfc1769 for details on SNTP */

/* constants */
/* {{{ */

#define SNTP_MODE_CLIENT 3
#define SNTP_MODE_SERVER 4
#define SNTP_MODE_BROADCAST 5

#define SNTP_VERSION 3

#define SNTP_UDP_PORT 123


/* reference dates, sntp timestamps are in seconds since 0h on 1900/01/01, which was a monday */
#define SNTP_REFERENCE_YEAR 1900    /* in YYYY */
#define SNTP_REFERENCE_DOW 1        /* 0 sunday, 1 monday, ... */

/* connection states */
#define SNTP_STATE_WAIT 0       /* connection is waiting for response */
#define SNTP_STATE_RESPONSE 1   /* response has been received */

/* timeout, after which the request is retransmitted */
#define SNTP_TIMEOUT 100

/* }}} */

/* structures */
/* {{{ */

struct sntp_timestamp_t {
    uint32_t seconds;
    uint32_t fraction;
};

struct sntp_header_t {
    uint8_t mode:3;
    uint8_t version:3;
    uint8_t leap_indicator:2;

    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_identifier;
    struct sntp_timestamp_t reference_timestamp;
    struct sntp_timestamp_t originate_timestamp;
    struct sntp_timestamp_t receive_timestamp;
    struct sntp_timestamp_t transmit_timestamp;
    /* 96 bit authenticator -- optional */
};

/* }}} */

/* global variables */
extern uip_ipaddr_t sntp_server;

/* prototypes */

/** send an sntp request */
void sntp_synchronize(void);
void sntp_send_request(void);
void sntp_handle_conn(void);

#endif
