/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
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

#include "../uip/uip.h"
#include "../config.h"
#include "../sensormodul/sensormodul.h"
#include "sensormodul_net.h"

#ifdef SENSORMODUL_SUPPORT

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))
#define STATS (uip_udp_conn->appstate.sensormodul)

uip_udp_conn_t *sensormodul_conn;

void 
sensormodul_net_init(void)
{
	uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);
	
  sensormodul_conn = uip_udp_new(&ip, 0, sensormodul_net_main);
	
  if(! sensormodul_conn) 
		return;					/* keine udp connection, tschuess !? */
	
  uip_udp_bind(sensormodul_conn, HTONS(SENSORMODUL_PORT));

	// Inititialisierung
  sensormodul_core_init(sensormodul_conn);
	
}

void
sensormodul_net_main(void)
{
  if (uip_poll()) 
    sensormodul_core_periodic();
  if (uip_newdata())
  {
    uip_udp_conn_t return_conn;
    if ( uip_datalen() <= SENSORMODUL_LCDTEXTLEN )
      sensormodul_setlcdtext(uip_appdata, uip_len);
    
    uip_ipaddr_copy(return_conn.ripaddr, BUF->srcipaddr);
    return_conn.rport = BUF->srcport;
    return_conn.lport = HTONS(SENSORMODUL_PORT);

    uip_send (&STATS, sizeof(struct sensormodul_datas_t));
    
    uip_udp_conn = &return_conn;
    /* Send immediately */
    uip_process(UIP_UDP_SEND_CONN);
    fill_llh_and_transmit();
    uip_slen = 0;
  }
}

#endif
