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
#include <stdlib.h>

#include "../uip/uip.h"
#include "../net/tftp_net.h"
#include "../crypto/skipjack.h"
#include "../eeprom.h"
#include "tftp.h"

#ifdef BOOTLOADER_SUPPORT

/* Define if you want to support firmware upload only. */
#undef  TFTP_UPLOAD_ONLY

/* Define if you want to temporarily disable firmware flashing. */
#undef  TFTP_DEBUG_DO_NOT_FLASH

/* defined in `timer.c' */
extern uint8_t bootload_delay;


/*
 * raw access to the packet buffer ...
 */
#define BUF ((struct uip_udpip_hdr *)&uip_appdata[-UIP_IPUDPH_LEN])


static void
flash_page(uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;

#ifdef TFTP_DEBUG_DO_NOT_FLASH
    return;
#endif

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
#ifdef SKIPJACK_SUPPORT
    unsigned char key[10] = CONF_TFTP_KEY;
#endif

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
#ifndef TFTP_UPLOAD_ONLY
    /*
     * streaming data back to the client (download) ...
     */
    case 1: /* read request */
	uip_udp_conn->appstate.tftp.download = 1;
	uip_udp_conn->appstate.tftp.transfered = 0;
	uip_udp_conn->appstate.tftp.finished = 0;

        bootload_delay = 0;                      /* Stop bootloader. */
    
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
            bootload_delay = CONF_BOOTLOAD_DELAY;    /* Restart bootloader. */
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
#if defined(SKIPJACK_SUPPORT)
	    pk->u.data.data[i + 8] = 
#else
	    pk->u.data.data[i] = 
#endif            
              pgm_read_byte_near(base + i);

#if defined(SKIPJACK_SUPPORT)
        for(i = 0; i < 8; i ++) {
            /* prepend 8 bytes IV */
            pk->u.data.data[i] = rand() & 0xFF;

            /* prepare to append CBC-MAC, i.e. initialize to zero,
             * so we can XOR the CBC carry and afterwards encrypt.  */
            pk->u.data.data[i + 512 + 8] = 0;
        }

        /* perform skipjack-cbc encryption:
         * leave the first block (iv) untouched,
         * encrypt blocks 1..64 (data)
         * then encrypt block 65 (cbc-mac) */
        for(i = 1; i <= 65; i ++) {
            /* carry cbc forward (xor) */
            for(int j = 0; j < 8; j ++) {
                pk->u.data.data[(i << 3) + j] ^= 
                  pk->u.data.data[(i << 3) + j - 8];
            }

            /* encrypt data */
            skipjack_enc(&pk->u.data.data[i << 3], key);
        }

        uip_udp_send(4 + 16 + 512);
#else /* !SKIPJACK_SUPPORT */
	uip_udp_send(4 + 512);
#endif

	uip_udp_conn->appstate.tftp.transfered ++;
	break;
#endif /* not TFTP_UPLOAD_ONLY */
	
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
        bootload_delay = 0;                      /* Stop bootloader. */

	if(uip_udp_conn->appstate.tftp.download != 0)
	    goto error_out;

	if(HTONS(pk->u.ack.block) < uip_udp_conn->appstate.tftp.transfered)
	    goto error_out;                     /* too early */

	if(HTONS(pk->u.ack.block) == uip_udp_conn->appstate.tftp.transfered)
	    goto send_ack;			/* already handled */

	if(HTONS(pk->u.ack.block) > uip_udp_conn->appstate.tftp.transfered + 1)
	    goto error_out;			/* too late */

#ifdef SKIPJACK_SUPPORT
        /* the packet looks like this:
         *   0..3    <packet preamble>
         *   4..11   IV         <- pk->u.data.data[0]
         *  12..522  DATA (maybe shorter) --,-  "decrypted"
         * 523..530  CBC-MAC              --'
         */
        for(i = 1; i < (uip_datalen() - 4) >> 3; i ++) {
            /* decrypt block (we actually encrypted, since used decrypt) */
            unsigned char buf[8];
            memmove(buf, pk->u.data.data + (i << 3), 8);
            skipjack_enc(buf, key);

            /* now XOR the decrypted data onto the block before,
             * which is still encrypted, i.e. our "cbc carry" */
            for(int j = 0; j < 8; j ++) 
                pk->u.data.data[(i << 3) + j - 8] ^= buf[j];
        }

        uip_datalen() -= 16;   /* lie about packet len */

        /* verify our CBC-MAC (XOR'd to zeroes), which should reside right
         * after the last datablock, which we've moved 8 bytes to the left */
        for(i = 0; i < 8; i ++) 
            if(pk->u.data.data[i + uip_datalen() - 4])
                goto error_out;

#endif /* SKIPJACK_SUPPORT */

	base = 512 * (HTONS(pk->u.ack.block) - 1);

	for(i = uip_datalen() - 4; i < 512; i ++)
	    pk->u.data.data[i] = 0xFF;	        /* EOF reached, init rest */

	for(i = 0; i < 512 / SPM_PAGESIZE; i ++)
	    flash_page(base + i * SPM_PAGESIZE, 
		       pk->u.data.data + i * SPM_PAGESIZE);

	if(uip_datalen() < 512 + 4) {
	    uip_udp_conn->appstate.tftp.finished = 1;

#           ifdef TFTPOMATIC_SUPPORT
            bootload_delay = 1;                      /* ack, then start app */
#           else
            bootload_delay = CONF_BOOTLOAD_DELAY;    /* Restart bootloader. */
#           endif
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


#endif /* BOOTLOADER_SUPPORT */
