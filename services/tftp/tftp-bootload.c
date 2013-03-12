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

#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <util/atomic.h>

#include "protocols/uip/uip.h"
#include "core/eeprom.h"
#include "core/mbr.h"
#include "core/debug.h"
#include "tftp.h"
#include "tftp_net.h"
#include "core/global.h"
#include "core/util/app_crc.h"


/* Define if you want to temporarily disable firmware flashing. */
#undef  TFTP_DEBUG_DO_NOT_FLASH

/*
 * raw access to the packet buffer ...
 */
#define BUF ((struct uip_udpip_hdr *)&((char *)uip_appdata)[-UIP_IPUDPH_LEN])

#if (MCU == atmega1284p) && (SPM_PAGESIZE != 256)
#warning Invalid SPM_PAGESIZE
#undef SPM_PAGESIZE
#define SPM_PAGESIZE 256
#endif
#define TFTP_BLOCK_SIZE 512
#if FLASHEND > UINT16_MAX
typedef uint32_t flash_base_t;
#define __pgm_read_byte pgm_read_byte_far
#else
typedef uint16_t flash_base_t;
#define __pgm_read_byte pgm_read_byte_near
#endif


static void
flash_page(uint32_t page, uint8_t * buf)
{
  uint16_t i;

#ifdef TFTP_DEBUG_DO_NOT_FLASH
  return;
#endif

  for (i = 0; i < SPM_PAGESIZE; i++)
    if (buf[i] != __pgm_read_byte(page + i))
      goto commit_changes;
  return;                               /* no changes */

commit_changes:
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    eeprom_busy_wait();

    boot_page_erase(page);
    boot_spm_busy_wait();

    for (i = 0; i < SPM_PAGESIZE; i += 2)
    {
      uint16_t w = *buf++;              /* set up little-endian word. */
      w += (*buf++) << 8;
      boot_page_fill(page + i, w);
    }

    boot_page_write(page);
    boot_spm_busy_wait();

    boot_rww_enable();                  /* reenable RWW-section again. */
  }
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
  uint16_t i;
  flash_base_t base;
  struct tftp_hdr *pk = uip_appdata;

  switch (HTONS(pk->type))
  {
#ifdef TFTP_UPLOAD_SUPPORT
      /*
       * streaming data back to the client (upload) ...
       */
    case 1:                             /* read request */
      uip_udp_conn->appstate.tftp.download = 1;
      uip_udp_conn->appstate.tftp.transfered = 0;
      uip_udp_conn->appstate.tftp.finished = 0;

      bootload_delay = 0;               /* stop bootloader. */
      goto send_data;

    case 4:                             /* acknowledgement */
      if (uip_udp_conn->appstate.tftp.download != 1)
        goto error_out;

      if (HTONS(pk->u.ack.block) < uip_udp_conn->appstate.tftp.transfered
          || (HTONS(pk->u.ack.block)
              > uip_udp_conn->appstate.tftp.transfered + 1))
        goto error_out;                 /* ack out of order */

      uip_udp_conn->appstate.tftp.transfered = HTONS(pk->u.ack.block);
    send_data:
      if (uip_udp_conn->appstate.tftp.finished)
      {
        bootload_delay = CONF_BOOTLOAD_DELAY;   /* restart bootloader. */
        return;                         /* nothing more to do */
      }

      pk->type = HTONS(3);              /* data packet */
      pk->u.data.block = HTONS(uip_udp_conn->appstate.tftp.transfered + 1);

      base = (flash_base_t) TFTP_BLOCK_SIZE *
        (flash_base_t) (uip_udp_conn->appstate.tftp.transfered);

      /* base overflowed ! */
#if FLASHEND == UINT16_MAX
      if (uip_udp_conn->appstate.tftp.transfered && base == 0)
#else
      if (base > FLASHEND)
#endif
      {
        uip_udp_send(4);        /* send empty packet to finish transfer */
        uip_udp_conn->appstate.tftp.finished = 1;
        return;
      }

      for (i = 0; i < TFTP_BLOCK_SIZE; i++)
        pk->u.data.data[i] = __pgm_read_byte(base + i);

      uip_udp_send(4 + TFTP_BLOCK_SIZE);
      uip_udp_conn->appstate.tftp.transfered++;
      break;
#endif /* TFTP_UPLOAD_SUPPORT */

      /*
       * streaming data from the client (firmware download) ...
       */
    case 2:                             /* write request */
      uip_udp_conn->appstate.tftp.download = 0;
      uip_udp_conn->appstate.tftp.transfered = 0;
      uip_udp_conn->appstate.tftp.finished = 0;

      pk->u.ack.block = HTONS(0);
      goto send_ack;

    case 3:                             /* data packet */
      bootload_delay = 0;               /* stop bootloader. */

      if (uip_udp_conn->appstate.tftp.download != 0)
        goto error_out;

      if (HTONS(pk->u.ack.block) < uip_udp_conn->appstate.tftp.transfered)
        goto error_out;                 /* too early */

      if (HTONS(pk->u.ack.block) == uip_udp_conn->appstate.tftp.transfered)
        goto send_ack;                  /* already handled */

      if (HTONS(pk->u.ack.block) > uip_udp_conn->appstate.tftp.transfered + 1)
        goto error_out;                 /* too late */

      base = (flash_base_t) TFTP_BLOCK_SIZE *
        (flash_base_t) (HTONS(pk->u.ack.block) - 1);

      for (i = uip_datalen() - 4; i < TFTP_BLOCK_SIZE; i++)
        pk->u.data.data[i] = 0xFF;      /* EOF reached, init rest */

      debug_putchar('.');

#ifdef TFTP_CRC_SUPPORT
      /* only flash when we are receiving an application binary */
      if (!uip_udp_conn->appstate.tftp.verify_crc)
#endif
      {
        for (i = 0; i < TFTP_BLOCK_SIZE / SPM_PAGESIZE; i++)
          flash_page(base + i * SPM_PAGESIZE,
              pk->u.data.data + i * SPM_PAGESIZE);
      }

      /* last packet in sequence */
      if (uip_datalen() < TFTP_BLOCK_SIZE + 4)
      {
        uip_udp_conn->appstate.tftp.finished = 1;

#ifdef TFTP_CRC_SUPPORT
        if (status.verify_tftp_crc_content)
        {
          uint16_t crc = calc_application_crc();
          if (pk->u.data.data[0] != (crc & 0x00ff) ||
              pk->u.data.data[1] != (crc >> 8))
          {
            pk->u.error.msg[1] = (crc & 0x00ff);
            pk->u.error.msg[2] = (crc >> 8);
            goto error_out;
          }
        }
        else
        {
          if (uip_udp_conn->appstate.tftp.verify_crc)
          {
            debug_putstr("\nCRC OK\n");
          }
          else
#else
        {
#endif
          {
#ifdef MBR_SUPPORT
            mbr_config.flashed = 1;
            write_mbr();
            debug_putstr("\nApp flashed\n");
#endif
          }
        }
        bootload_delay = 1;             /* give time to ack packet,
                                         * then start app */
      }

      uip_udp_conn->appstate.tftp.transfered = HTONS(pk->u.ack.block);

    /* send ack */
    send_ack:
      pk->type = HTONS(4);
      uip_udp_send(4);
      break;

    /* handle errors */
    error_out:
    case 5:                             /* error */
    default:
      pk->type = HTONS(5);              /* data packet */
      pk->u.error.code = HTONS(0);      /* undefined error code */
      pk->u.error.msg[0] = 0;           /* yes, really expressive */
      uip_udp_send(7);

#ifdef TFTP_CRC_SUPPORT
      if(uip_udp_conn->appstate.tftp.verify_crc)
        /* there was no matching crc file on the tftp server.
         * so we will try to get the application binary.
         * the initial request connection should already be unbound. */
        tftp_fire_tftpomatic(&uip_udp_conn->ripaddr,
            uip_udp_conn->appstate.tftp.filename, 0);
#endif
  }
}
