/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef _TFTP_H
#define _TFTP_H

#include "../config.h"
#ifdef TFTP_SUPPORT

#include <stdint.h>

/* tftp packet header */
struct tftp_hdr {
    uint16_t type;
    
    union {
	char raw[0];
	
	struct {
	    uint16_t block;
	    char data[0];
	} data;

	struct {
	    uint16_t block;
	} ack;

	struct {
	    uint16_t code;
	    char msg[1];
	} error;
    } u;
};


/* prototypes */
void tftp_handle_packet(void);


#if defined(TFTPOMATIC_SUPPORT) || defined(BOOTP_SUPPORT)
inline static void
tftp_fire_tftpomatic(uip_ipaddr_t *ip, const unsigned char *filename) {
  uip_udp_conn_t *tftp_req_conn = 
    uip_udp_new(ip, HTONS(TFTP_PORT), tftp_net_main);

  if(! tftp_req_conn) 
      return;					/* dammit. */

  uip_udp_bind(tftp_req_conn, HTONS(TFTP_ALT_PORT));
  tftp_req_conn->appstate.tftp.fire_req = 1;
  memcpy(tftp_req_conn->appstate.tftp.filename, filename,
         TFTP_FILENAME_MAXLEN);
  tftp_req_conn->appstate.tftp.filename[TFTP_FILENAME_MAXLEN - 1] = 0;


  /* create suitable tftp receiver */
  uip_udp_conn_t *tftp_recv_conn =
    uip_udp_new(ip, 0, tftp_net_main);

  if(! tftp_recv_conn) 
      return;					/* dammit. */

  uip_udp_bind(tftp_recv_conn, HTONS(TFTP_ALT_PORT));
  tftp_recv_conn->appstate.tftp.download = 0;
  tftp_recv_conn->appstate.tftp.transfered = 0;
  tftp_recv_conn->appstate.tftp.finished = 0;
  tftp_recv_conn->appstate.tftp.bootp_image = 1;
}
#endif /* TFTPOMATIC_SUPPORT || BOOTP_SUPPORT */

#endif /* TFTP_SUPPORT */
#endif /* _TFTP_H */
