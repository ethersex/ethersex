/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "speed_parser.h"
#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
/* we can manipulate crons via the ecmd speed protocol */
#include "services/cron/cron.h"
/* we can manipulate stella via the ecmd speed protocol */
#include "services/stella/stella.h"
/* we want to send via uip */
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/uip/uip_arp.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
ecmd_speed_error()
{
	#ifdef DEBUG
	debug_printf("Ecmd_speed parse error\n");
	#endif
}

void
ecmd_speed_parse(char* buf, uint8_t len)
{
	uint16_t size;
	while (len>0)
	{
		char cmd = *buf;
		// GETTER and ACTIONS
		size = 0;
		char* dataout = uip_appdata+sizeof(ecmd_speed_response);
		switch (cmd)
		{
			#ifdef ECMD_SPEED_REBOOT
			case ECMDS_ACTION_RESET:
				status.request_reset = 1;
				#ifdef UIP_SUPPORT
				uip_close();
				#endif
				break;
			case ECMDS_ACTION_BOOTLOADER:
				status.request_bootloader = 1;
				#ifdef UIP_SUPPORT
				uip_close();
				#endif
				break;
				#endif
			case ECMDS_GET_PROTOCOL_VERSION:
				size = 1;
				dataout[0] = ECMD_SPEED_PROTOCOL_VERSION;
				break;
			case ECMDS_GET_PROTOCOL_COMBAT_VERSION:
				size = 1;
				dataout[0] = ECMD_SPEED_PROTOCOL_COMBAT_VERSION;
				break;
			case ECMDS_GET_ETHERSEX_VERSION:
				size = 2;
				dataout[0] = VERSION_MAJOR;
				dataout[1] = VERSION_MINOR;
				break;
			case ECMDS_GET_ETHERSEX_MAC_IP_GW_MASK:
				break;
				#ifdef STELLA_SUPPORT
			case ECMDS_GET_STELLA_COLORS:
				size = stella_output_channels(dataout);
				break;
			case ECMDS_GET_STELLA_FADE_FUNC_STEP:
				size = 2;
				dataout[0] = stella_fade_func;
				dataout[1] = stella_fade_step;
				break;
				#ifdef STELLA_MOODLIGHT
			case ECMDS_GET_STELLA_MOODLIGHT_DATA:
				size = 2;
				dataout[0] = stella_moodlight_mask;
				dataout[1] = stella_moodlight_threshold;
				break;
				#endif //STELLA_MOODLIGHT
				#endif //STELLA_SUPPORT
				#ifdef CRON_SUPPORT
			case ECMDS_GET_CRON_COUNT:
				size = 1;
				dataout[0] = cron_jobs();
				#ifdef DEBUG_ECMD_SPEED
				debug_printf("COUNTCRON %u\n", dataout[0]);
				#endif
				break;
			case ECMDS_GET_CRONS:
				size = cron_output(dataout, UIP_APPDATA_SIZE*0.75);
				break;
				#endif //CRON_SUPPORT
			case ECMDS_GET_PORTPINS:
				size = IO_PORTS;
				uint8_t c;
				for (c=0;c< IO_PORTS;++c)
					dataout[c] = vport[c].read_port(c);
				break;
			case ECMDS_JUMP_TO_FUNCTION:
				break;
		}

		if (size)
		{
			// response header
			ecmd_speed_response* dataout_header = uip_appdata;
			dataout_header->id = 'S';
			dataout_header->cmd = cmd;
			dataout_header->size = htons(size);
			size += sizeof(ecmd_speed_response);
			// send
			uip_udp_send(size);

			/* Send the packet */
			uip_udp_conn_t conn;
			uip_ipaddr_t addr;
			#if 1
			uip_ipaddr_copy(addr, BUF->srcipaddr);
			#else
			uip_ipaddr(&addr, 255,255,255,255);
			#endif
			uip_ipaddr_copy(conn.ripaddr, addr);
			conn.rport = BUF->srcport;
			conn.lport = HTONS(ECMD_UDP_PORT);

			uip_udp_conn = &conn;

			/* Send immediately */
			uip_process(UIP_UDP_SEND_CONN);
			router_output();

			uip_slen = 0;
			return;
		}

		// SETTER
		size = 1;
		switch (cmd)
		{
			#ifdef ECMD_SPEED_ENETCMDS
			case ECMDS_SET_ETHERSEX_MAC:
				if (len<sizeof(struct uip_eth_addr)+1) return;
				struct uip_eth_addr new_mac;
				memcpy(&new_mac, buf+1, sizeof(struct uip_eth_addr));
				eeprom_save(mac, &new_mac, sizeof(struct uip_eth_addr));
				eeprom_update_chksum();
				size+=sizeof(struct uip_eth_addr);
				break;
			case ECMDS_SET_ETHERSEX_IP:
				if (len<sizeof(uip_ipaddr_t)+1) return;
				uip_ipaddr_t hostaddr;
				memcpy(&hostaddr, buf+1, sizeof(uip_ipaddr_t));
				eeprom_save(ip, &hostaddr, IPADDR_LEN);
				eeprom_update_chksum();
				size+=sizeof(uip_ipaddr_t);
				break;
			case ECMDS_SET_ETHERSEX_GW_IP:
				if (len<sizeof(uip_ipaddr_t)+1) return;
				uip_ipaddr_t gwaddr;
				memcpy(&gwaddr, buf+1, sizeof(uip_ipaddr_t));
				eeprom_save(gateway, &gwaddr, IPADDR_LEN);
				eeprom_update_chksum();
				size+=sizeof(uip_ipaddr_t);
				break;
			case ECMDS_SET_ETHERSEX_NETMASK:
				if (len<sizeof(uip_ipaddr_t)+1) return;
				uip_ipaddr_t new_netmask;
				memcpy(&new_netmask, buf+1, sizeof(uip_ipaddr_t));
				eeprom_save(netmask, &new_netmask, IPADDR_LEN);
				eeprom_update_chksum();
				size+=sizeof(uip_ipaddr_t);
				break;
			#endif
			case ECMDS_SET_ETHERSEX_EVENTMASK:
				break;
			#ifdef STELLA_SUPPORT
			case ECMDS_SET_STELLA_INSTANT_COLOR:
				stella_setValue(STELLA_SET_IMMEDIATELY, buf[1], buf[2]);
				size+=2;
				break;
			case ECMDS_SET_STELLA_FADE_COLOR:
				stella_setValue(STELLA_SET_FADE, buf[1], buf[2]);
				size+=2;
				break;
			case ECMDS_SET_STELLA_FLASH_COLOR:
				stella_setValue(STELLA_SET_FLASHY, buf[1], buf[2]);
				size+=2;
				break;
			case ECMDS_SET_STELLA_FADE_FUNC:
				if (buf[1] < FADE_FUNC_LEN)
					stella_fade_func = buf[1];
				size+=1;
				break;
			case ECMDS_SET_STELLA_FADE_STEP:
				stella_fade_step = buf[1];
				size+=1;
				break;
			case ECMDS_SET_STELLA_SAVE_TO_EEPROM:
				stella_storeToEEROM();
				break;
			case ECMDS_SET_STELLA_LOAD_FROM_EEPROM:
				stella_loadFromEEROM();
				break;
			#ifdef STELLA_MOODLIGHT
			case ECMDS_SET_STELLA_MOODLIGHT_MASK:
				stella_moodlight_mask = buf[1];
				size+=1;
				break;
			case ECMDS_SET_STELLA_MOODLIGHT_THRESHOLD:
				stella_moodlight_threshold = buf[1];
				size+=1;
				break;
			#endif //STELLA_MOODLIGHT
			#endif //STELLA_SUPPORT
			#ifdef CRON_SUPPORT
			case ECMDS_SET_CRON_REMOVE:
				cron_jobrm(cron_getjob(buf[1]));
				size+=1;
				break;
			case ECMDS_SET_CRON_ADD:
				if (len<cron_event_size) return;
				size += cron_input(buf+1);
				break;
			#endif
			case ECMDS_SET_PORTPIN: // port, pin, on/off
				if (len<3) return;
				uint8_t port = buf[1];
				if (buf[3])
					vport[port].write_port(port, vport[port].read_port(port) | _BV(buf[2]));
				else
					vport[port].write_port(port, vport[port].read_port(port) & ~_BV(buf[2]));
				size+=3;
				break;
			default:
				break;
		}
		buf += (uint8_t)size;
		len -= (uint8_t)size;
	}
}
