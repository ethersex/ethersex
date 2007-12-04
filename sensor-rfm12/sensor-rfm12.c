/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
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
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../net/sensor-rfm12_state.h"
#include "../uip/uip.h"
#include "../config.h"
#include "sensor-rfm12.h"

#ifdef SENSOR_RFM12_SUPPORT

#define STATS (uip_udp_conn->appstate.sensor-rfm12)

/*
 * direkter zugriff zum packet buffer
 */
#define BUF ((struct uip_udpip_hdr *)&uip_appdata[-UIP_IPUDPH_LEN])

void 
sensor-rfm12_core_init(struct uip_udp_conn *sensor-rfm12_conn)
{

}

void 
sensor-rfm12_core_periodic(void)
{

}

void 
sensor-rfm12_core_newdata(void)
{
	
    //struct sensor-rfm12_request_t *REQ = uip_appdata;
		/*
		* ueberschreiben der connection info. 
		* port und adresse auf den remotehost begrenzen
		*/
    /*
			uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
			uip_udp_conn->rport = BUF->srcport;
    */


}

#endif
