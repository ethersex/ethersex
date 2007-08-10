/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <string.h>

#include "../uip/uip.h"
#include "../net/tftp_net.h"
#include "tftp.h"

/*
 * reset vectors
 */
extern void (*jump_to_application)(void);
extern void (*jump_to_bootloader)(void);


/*
 * raw access to the packet buffer ...
 */
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])


static void
flash_page(uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;

    for(i = 0; i < SPM_PAGESIZE; i ++)
	if(buf[i] != pgm_read_byte_near(page + i))
	    goto commit_changes;
    return;					/* no changes */

 commit_changes:
    /* Disable interrupts. */
    sreg = SREG;
    cli();
    
    eeprom_busy_wait();

    boot_page_erase(page);
    boot_spm_busy_wait();

    for(i = 0; i < SPM_PAGESIZE; i += 2) {
	/* Set up little-endian word. */
	uint16_t w = *buf++;
	w += (*buf++) << 8;
        
	boot_page_fill (page + i, w);
    }

    boot_page_write (page);
    boot_spm_busy_wait();

    /* Reenable RWW-section again. */
    boot_rww_enable ();

    /* Re-enable interrupts (if they were ever enabled). */
    SREG = sreg;
}


void
tftp_handle_packet(void)
{
    /*
     * overwrite udp connection information (i.e. take from incoming packet)
     */
    uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
    uip_udp_conn->rport = BUF->srcport;


    /*
     * care for incoming tftp packet now ...
     */
    uint16_t i, base;
    struct tftp_hdr *pk = uip_appdata;

    switch(HTONS(pk->type)) {
    /*
     * streaming data back to the client (download) ...
     */
    case 1: /* read request */
	uip_udp_conn->appstate.tftp.download = 1;
	uip_udp_conn->appstate.tftp.transfered = 0;
	uip_udp_conn->appstate.tftp.finished = 0;

	goto send_data;

    case 4: /* acknowledgement */
	if(uip_udp_conn->appstate.tftp.download != 1)
	    goto error_out;

	if(HTONS(pk->u.ack.block) < uip_udp_conn->appstate.tftp.transfered
	   || (HTONS(pk->u.ack.block) >
	       uip_udp_conn->appstate.tftp.transfered + 1))
	    goto error_out; /* ack out of order */

	uip_udp_conn->appstate.tftp.transfered = HTONS(pk->u.ack.block);

    send_data:
	if(uip_udp_conn->appstate.tftp.finished) {
	    return;                                  /* nothing more to do */
	}

	pk->type = HTONS(3); /* data packet */
	pk->u.data.block = HTONS(uip_udp_conn->appstate.tftp.transfered + 1);

	base = 512 * uip_udp_conn->appstate.tftp.transfered;

#if FLASHEND == 0xFFFF
	if(uip_udp_conn->appstate.tftp.transfered
	   && base == 0)     /* base overflowed ! */
#else
	if(base > FLASHEND) 
#endif
	{
	    uip_udp_send(4); /* send empty packet to finish transfer */
	    uip_udp_conn->appstate.tftp.finished = 1;
	    return;
	}

	for(i = 0; i < 512; i ++)
	    pk->u.data.data[i] = pgm_read_byte_near(base + i);

	uip_udp_send(4 + 512);
	uip_udp_conn->appstate.tftp.transfered ++;
	break;

	
    /*
     * streaming data from the client (firmware upload) ...
     */
    case 2: /* write request */
	uip_udp_conn->appstate.tftp.download = 0;
	uip_udp_conn->appstate.tftp.transfered = 0;
	uip_udp_conn->appstate.tftp.finished = 0;

	pk->u.ack.block = HTONS(0);
	goto send_ack;

    case 3: /* data packet */
	if(uip_udp_conn->appstate.tftp.download != 0)
	    goto error_out;

	if(HTONS(pk->u.ack.block) < uip_udp_conn->appstate.tftp.transfered)
	    goto error_out;                     /* too early */

	if(HTONS(pk->u.ack.block) == uip_udp_conn->appstate.tftp.transfered)
	    goto send_ack;			/* already handled */

	if(HTONS(pk->u.ack.block) > uip_udp_conn->appstate.tftp.transfered + 1)
	    goto error_out;			/* too late */
	
	base = 512 * (HTONS(pk->u.ack.block) - 1);

	for(i = uip_datalen() - 4; i < 512; i ++)
	    pk->u.data.data[i] = 0xFF;	        /* EOF reached, init rest */

	for(i = 0; i < 512 / SPM_PAGESIZE; i ++)
	    flash_page(base + i * SPM_PAGESIZE, 
		       pk->u.data.data + i * SPM_PAGESIZE);

	if(uip_datalen() < 512 + 4) {
	    uip_udp_conn->appstate.tftp.finished = 1;
	}

	uip_udp_conn->appstate.tftp.transfered = HTONS(pk->u.ack.block);
	
    send_ack:
	pk->type = HTONS(4);
	uip_udp_send(4);              /* send ack */
	break;

    /*
     * protocol errors
     */
    error_out:
    case 5: /* error */
    default:
	pk->type = HTONS(5);          /* data packet */
	pk->u.error.code = HTONS(0);  /* undefined error code */
	pk->u.error.msg[0] = 0;       /* yes, really expressive */
	uip_udp_send(5);
	break;
    }
}



