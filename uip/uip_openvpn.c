/* vim:fdm=marker ts=4 et ai
 * {{{
 *
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
 }}} */

#include "../config.h"
#ifdef OPENVPN_SUPPORT

/* We're now compiling the outer side of the uIP stack */
#define OPENVPN_OUTER
#define STACK_NAME(a) openvpn_ ## a

#include "uip_openvpn.h"

/* We're set to compile multi stack now ... */
#include "uip.c"

/* for raw access to the packet buffer */
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#ifdef CAST5_SUPPORT
#include "../crypto/cast5.h"
static unsigned char *key = CONF_OPENVPN_KEY;
static cast5_ctx_t ctx;

/* Decrypt the cast5 encrypted OpenVPN packet and verify the packet
   id.  Return non-zero on error. */
int
openvpn_decrypt_and_verify (void)
{
  unsigned char buf[8];

  unsigned char *cbc_carry_this =
    uip_appdata + OPENVPN_HMAC_LLH_LEN;	/* initial IV. */
  unsigned char *cbc_carry_next = buf;
  
  for (unsigned char *ptr = uip_appdata + 8 + OPENVPN_HMAC_LLH_LEN;
       ptr < ((unsigned char *) uip_appdata) + uip_len;
       ptr += 8)
    {
      /* store cbc-carry for next round */
      memcpy (cbc_carry_next, ptr, 8);

      cast5_dec (&ctx, ptr);

      /* apply cbc-carry of this round */
      for (int i = 0; i < 8; i ++)
	ptr[i] ^= cbc_carry_this[i];

      /* exchange pointers */
      unsigned char *tmp = cbc_carry_this;
      cbc_carry_this = cbc_carry_next;
      cbc_carry_next = tmp;
    }

  /* verify packet-id */
  uint32_t *packet_id = (uint32_t *) (uip_appdata + OPENVPN_HMAC_LLH_LEN + 8);
  if (HTONL(packet_id[1]) <= uip_udp_conn->appstate.openvpn.seen_timestamp)
    {
      if (HTONL(packet_id[0]) <= uip_udp_conn->appstate.openvpn.seen_seqno)
	return 1;
    }

  uip_udp_conn->appstate.openvpn.seen_seqno = HTONL(packet_id[0]);
  uip_udp_conn->appstate.openvpn.seen_timestamp = HTONL(packet_id[1]);

  return 0;
}

/* The length in openvpn_slen is already including the extra
   encryption/hmac header. */
void
openvpn_encrypt (void)
{
  unsigned char *encrypt_start =
    &uip_buf[OPENVPN_LLH_LEN + OPENVPN_HMAC_LLH_LEN];

  /* Do padding. */
  unsigned char pad_char = 8 - (openvpn_slen % 8);
  do
    ((unsigned char *) uip_sappdata)[openvpn_slen ++] = pad_char;
  while (openvpn_slen % 8);

  /* Generate IV. */
  unsigned char *ptr;
  for(ptr = encrypt_start; ptr < encrypt_start + 8; ptr ++)
    *ptr = rand() & 0xFF;

  /* Fill packet-id. */
  uint32_t *packet_id = (uint32_t *) (encrypt_start + 8);
  packet_id[0] = HTONL(uip_udp_conn->appstate.openvpn.next_seqno);
  packet_id[1] = 0;

  /* Increment sequence number. */
  uip_udp_conn->appstate.openvpn.next_seqno ++;

  /* Encrypt data. */
  for (ptr = encrypt_start + 8;
       ptr < ((unsigned char *) openvpn_sappdata) + openvpn_slen;
       ptr += 8)
    {
      /* apply cbc-carry forward */
      for (int i = 0; i < 8; i ++)
	ptr[i] ^= ptr[i - 8];

      cast5_enc (&ctx, ptr);
    }
}

#else /* !CAST5_SUPPORT */
#define openvpn_decrypt_and_verify()  0
#define openvpn_encrypt() do { (void) 0; } while(0)
#endif


#ifdef MD5_SUPPORT
#include "../crypto/md5.h"

void
openvpn_hmac_calc (unsigned char *dest, unsigned char *src, uint16_t len)
{
  const unsigned char *hmac_key = CONF_OPENVPN_HMAC_KEY;
  unsigned char buf[64];
  
  /* perform inner part of hmac */
  md5_ctx_t ctx_inner;
  md5_init (&ctx_inner);
  
  for (int i = 0; i < 16; i ++) buf[i] = hmac_key[i] ^ 0x36;
  for (int i = 16; i < 64; i ++) buf[i] = 0x36;
  
  md5_nextBlock (&ctx_inner, buf);
  md5_lastBlock (&ctx_inner, src, len << 3);

  /* perform outer part of hmac */
  md5_ctx_t ctx_outer;
  md5_init (&ctx_outer);

  for (int i = 0; i < 16; i ++) buf[i] = hmac_key[i] ^ 0x5c;
  for (int i = 16; i < 64; i ++) buf[i] = 0x5c;
  md5_nextBlock (&ctx_outer, buf);
  md5_lastBlock (&ctx_outer, (void *) &ctx_inner.a[0], 128);

  memmove (dest, (void *) &ctx_outer.a[0], 16);
}

int
openvpn_hmac_verify (void)
{
  unsigned char hmac_buf[16];
  openvpn_hmac_calc (hmac_buf, uip_appdata + 16, uip_len - 16);

  if (memcmp (hmac_buf, uip_appdata, 16))
    {
      /* hmac error */
      return 1;
    }

  return 0;
}

#define openvpn_hmac_create()					       \
  openvpn_hmac_calc (uip_buf + OPENVPN_LLH_LEN,			       \
		     uip_buf + OPENVPN_LLH_LEN + OPENVPN_HMAC_LLH_LEN, \
		     openvpn_slen - OPENVPN_HMAC_LLH_LEN)

#else /* !MD5_SUPPORT */
#define openvpn_hmac_verify() 0
#define openvpn_hmac_create() do { (void) 0; } while(0)
#endif


void 
openvpn_handle_udp (void)
{
  if (uip_udp_conn->lport != HTONS(OPENVPN_PORT))
    return;
  if (!uip_newdata ())
    return;

  /* Overwrite udp connection information (i.e. take from incoming packet). */
  uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
  uip_udp_conn->rport = BUF->srcport;

  if (openvpn_hmac_verify ())
    return;

  if (openvpn_decrypt_and_verify ())
    return;

  /* ``uip_len'' is the number of payload bytes (including
     hmac/encryption bits), however uip_process expects the number of
     bytes including the LLH. */
  uip_len += OPENVPN_LLH_LEN;

  /* Push data into inner uIP stack. */
  uip_stack_set_active (STACK_MAIN);
  mainstack_process (UIP_DATA);
  uip_stack_set_active (STACK_OPENVPN);

  if (! uip_len)
    return;			/* Inner stack hasn't created a
				   packet. */

  /* Make sure openvpn_process sends the data. */
  openvpn_slen = uip_len + OPENVPN_HMAC_LLH_LEN + OPENVPN_CRYPT_LLH_LEN;
  openvpn_encrypt ();
  openvpn_hmac_create ();
}


/* Prepare data from inner uIP stack to be sent out to the remote host,
   this is fill the IP and UDP headers of the outer stack part.  */
void
openvpn_process_out (void)
{
  /* uip_len is shared between both stacks.  uip_process (from the
     inner stack) has set it to the amount of data to be tunnelled
     (including TCP, etc. headers).  */
  if (! uip_len)
    return;			/* no data to be sent out. */

  uip_stack_set_active (STACK_OPENVPN);
  openvpn_slen = uip_len + OPENVPN_HMAC_LLH_LEN + OPENVPN_CRYPT_LLH_LEN;

  for (int i = 0; i < UIP_UDP_CONNS; i++)
    if (uip_udp_conn[i].callback == openvpn_handle_udp)
      {
	openvpn_udp_conn = &uip_udp_conn[i];
	break;
      }

  openvpn_encrypt ();
  openvpn_hmac_create ();
  openvpn_process (UIP_UDP_SEND_CONN);
}


void 
openvpn_init (void)
{
  uip_ipaddr_t ip;

#ifdef CAST5_SUPPORT
  cast5_init(&ctx, key, 128);
#endif

  /* Initialize OpenVPN stack IP config, if necessary. */
# if !UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)
  CONF_OPENVPN_IP4;
  uip_sethostaddr(ip);

  CONF_OPENVPN_IP4_NETMASK;
  uip_setnetmask(ip);

  CONF_OPENVPN_IP4_GATEWAY;
  uip_setdraddr(ip);
# endif /* not UIP_CONF_IPV6 and not BOOTP */

# if UIP_CONF_IPV6
  uip_setprefixlen(64);
  uip_ip6autoconfig(0xFE80, 0x0000, 0x0000, 0x0000);
# if UIP_CONF_IPV6_LLADDR
  uip_ipaddr_copy(uip_lladdr, uip_hostaddr);
# endif
# endif

  /* Create OpenVPN UDP listener. */
  uip_ipaddr_copy(&ip, all_ones_addr);

  uip_udp_conn_t *openvpn_conn = uip_udp_new(&ip, 0, openvpn_handle_udp);

  if(! openvpn_conn) 
    return;					/* dammit. */

  uip_udp_bind(openvpn_conn, HTONS(OPENVPN_PORT));

  openvpn_conn->appstate.openvpn.next_seqno = 1;
  openvpn_conn->appstate.openvpn.seen_seqno = 0;
  openvpn_conn->appstate.openvpn.seen_timestamp = 0;
}

#endif
