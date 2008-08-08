/*
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/pgmspace.h>

#include <string.h>
#include <stdlib.h>

#include "../uip/uip.h"
#include "../dataflash/df.h"
#include "../net/tftp_net.h"
#include "tftp.h"

#if 0 				/* Enable debuging with SYSLOG. */
#  include "../syslog/syslog.h"
#  define DEBUG(a...) syslog_sendf(a)
#else
#  define DEBUG(a...) do { } while (0)
#endif

#ifdef DATAFLASH_SUPPORT

/*
 * raw access to the packet buffer ...
 */
#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))




void
tftp_handle_packet(void)
{
    struct tftp_connection_state_t *state = &uip_udp_conn->appstate.tftp;
    /*
     * overwrite udp connection information (i.e. take from incoming packet)
     */
    uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
    uip_udp_conn->rport = BUF->srcport;

    /*
     * care for incoming tftp packet now ...
     */
    struct tftp_hdr *pk = uip_appdata;

    switch(HTONS(pk->type)) {
    /*
     * streaming data back to the client (download) ...
     */
    case 1:			/* read request */
	state->download = 1;
	state->transfered = 0;
	state->finished = 0;

	state->df_access = memcmp_P (pk->u.raw, PSTR ("df+"), 3) == 0;

	if (state->df_access == 0) {
	    state->fs_inode = fs_get_inode (&fs, pk->u.raw);

	    if (state->fs_inode == 0xffff)
		goto error_out;
	}
	else {
	    state->fs_inode = strtol (pk->u.raw + 3, NULL, 16);
	    if (state->fs_inode >= DF_PAGES)
		goto error_out;
	}

	goto send_data;

    case 4:			/* acknowledgement */
	if(state->download != 1)
	    goto error_out;

	if(HTONS(pk->u.ack.block) < state->transfered
	   || (HTONS(pk->u.ack.block) >
	       state->transfered + 1))
	    goto error_out;	/* ack out of order */

	state->transfered = HTONS(pk->u.ack.block);

    send_data:
	if(state->finished)
	    goto close_connection;

	pk->type = HTONS(3);	/* data packet */
	pk->u.data.block = HTONS(state->transfered + 1);

	if (state->df_access) {
	    if (state->transfered == 0) {
		df_flash_read (fs.chip, state->fs_inode, pk->u.data.data,
			       0, 512);
		uip_udp_send(4 + 512);
	    }
	    else {
		df_flash_read (fs.chip, state->fs_inode, pk->u.data.data,
			       512, DF_PAGESIZE - 512);
		uip_udp_send(4 + DF_PAGESIZE - 512);
		state->finished = 1;
	    }
	}
	else {			/* send file */
	    fs_size_t ret;
	    fs_size_t offset = state->transfered * 512;

	    ret = fs_read (&fs, state->fs_inode, pk->u.data.data, offset, 512);
	    DEBUG ("fs_read: o=%04lx, l=%04lx\n", offset, ret);
	    
	    if (ret < 0)
		goto error_out;
	    if (ret < 512)
		state->finished = 1;
	    uip_udp_send (4 + ret);
	}

	state->transfered ++;
	break;
	
    /*
     * streaming data from the client (firmware upload) ...
     */
    case 2:			/* write request */
	state->download = 0;
	state->transfered = 0;
	state->finished = 0;

	/* try to create the file, shouldn't hurt if it already exists */
	state->fs_inode = fs_create (&fs, pk->u.raw);
	state->fs_inode = fs_get_inode (&fs, pk->u.raw);

	if (state->fs_inode == 0xffff)
	    goto error_out;

	pk->u.ack.block = HTONS(0);
	goto send_ack;

    case 3:			/* data packet */
	if (state->download != 0)
	    goto error_out;

	if (HTONS (pk->u.ack.block) < state->transfered)
	    goto error_out;	/* too early */

	if (HTONS (pk->u.ack.block) == state->transfered)
	    goto send_ack;	/* already handled */

	if (HTONS (pk->u.ack.block) > state->transfered + 1)
	    goto error_out;	/* too late */

	fs_status_t ret;
	fs_size_t offset = 512 * (HTONS(pk->u.ack.block) - 1);

	ret = fs_write (&fs, state->fs_inode, pk->u.data.data,
			offset, uip_datalen () - 4);

	if (ret != FS_OK) {
	    DEBUG ("fs_write o=%04lx, l=%04x, r=%d\n", offset,
		   uip_datalen () - 4, ret);
	    goto error_out;
	}

	if (uip_datalen () < 512 + 4)
	    state->finished = 1;

	state->transfered = HTONS (pk->u.ack.block);
	
    send_ack:
	pk->type = HTONS (4);
	uip_udp_send (4);              /* send ack */

	if (state->finished)
	    goto close_connection;

	break;

    /*
     * protocol errors
     */
    error_out:
    case 5:			/* error */
    default:
	pk->type = HTONS(5);          /* data packet */
	pk->u.error.code = HTONS(0);  /* undefined error code */
	pk->u.error.msg[0] = 0;       /* yes, really expressive */
	uip_udp_send(5);

    close_connection:
	if (uip_slen) {
	    /* there's still data that has to be sent,
	       push it immediately. */
	    uip_process(UIP_UDP_SEND_CONN);
	    fill_llh_and_transmit();

	    uip_slen = 0;	/* don't send twice. */
	}

	/* Reset connection. */
	uip_ipaddr_copy(uip_udp_conn->ripaddr, all_ones_addr);
	uip_udp_conn->rport = 0;

	break;
    }
}


#endif /* DATAFLASH_SUPPORT */
