/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "config.h"
#include "snmp.h"
#include "snmp_net.h"

void
snmp_net_init(void)
{
  uip_udp_conn_t *conn;
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  if (!(conn = uip_udp_new(&ip, 0, snmp_net_main)))
    return;                     /* Couldn't bind socket */

  uip_udp_bind(conn, HTONS(SNMP_PORT));
}

const struct snmp_reaction *
snmp_find_reaction(uint8_t * oid, uint8_t oid_len,
                   struct snmp_varbinding *bind)
{
  const struct snmp_reaction *z;
  const char *obj_name;
  for (z = snmp_reactions;
       (obj_name = (const char *) pgm_read_word(&z->obj_name)) != NULL; z++)
  {
    uint8_t store_len = strlen_P(obj_name);
    uint8_t cmp_len, len;
    if (store_len > oid_len)
    {
      cmp_len = oid_len;
      len = 0;
    }
    else
    {
      cmp_len = store_len;
      len = oid_len - store_len;
    }

    if (memcmp_P(oid, obj_name, cmp_len) == 0)
    {
      bind->store_len = store_len;
      bind->len = len;
      bind->data = oid + store_len;
      return z;
    }
  }
  return NULL;
}

uint8_t
snmp_proc_bind(uint8_t req_type, uint8_t * oid, uint8_t oid_len,
               uint8_t * out)
{
  struct snmp_varbinding bind;
  const struct snmp_reaction *reaction;
  uint8_t ret;

  /* search reaction */
  if (oid_len == 0 && req_type == SNMP_TYPE_GETNEXTREQ)
  {
    reaction = snmp_reactions;
  }
  else
  {
    reaction = snmp_find_reaction(oid, oid_len, &bind);
  }
  if (reaction == NULL)
  {
    return 0;
  }

  /* add varbind to output */
  uint8_t *vb = out;
  *(out++) = SNMP_TYPE_SEQUENCE;
  *(out++) = 0;

  if (req_type == SNMP_TYPE_GETNEXTREQ)
  {
    /* try to find next sub OID of matching record */
    snmp_next_callback_t ncb =
      (snmp_next_callback_t) pgm_read_word(&reaction->ncb);
    if (ncb != NULL)
    {
      uint8_t *new = out + 2 + bind.store_len;
      bind.len = ncb(new, &bind);
      bind.data = new;
    }

    /* no next sub OID -> try next record */
    if (bind.len == 0)
    {
      reaction++;
      const char *obj_name =
        (const char *) pgm_read_word(&reaction->obj_name);
      if (obj_name == NULL)
      {
        return 0;
      }
      bind.store_len = strlen_P(obj_name);
      bind.len = 0;
      bind.data = out + 2 + bind.store_len;

      ncb = (snmp_next_callback_t) pgm_read_word(&reaction->ncb);
      if (ncb != NULL)
      {
        bind.len = ncb(bind.data, &bind);
      }
    }

    /* claculate OID lenhth */
    oid_len = bind.store_len + bind.len;

    /* add oid to output */
    const char *obj_name = (const char *) pgm_read_word(&reaction->obj_name);
    *(out++) = SNMP_TYPE_OID;
    *(out++) = oid_len;
    memcpy_P(out, obj_name, bind.store_len);
    out += oid_len;
  }
  else
  {
    /* check OID length */
    if (oid_len < bind.store_len)
    {
      return 0;
    }

    /* add oid to output */
    *(out++) = SNMP_TYPE_OID;
    *(out++) = oid_len;
    memcpy(out, oid, oid_len);
    out += oid_len;
  }

  /* process reaction */
  snmp_reaction_callback_t cb =
    (snmp_reaction_callback_t) pgm_read_word(&reaction->cb);
  void *data = (void *) pgm_read_word(&reaction->userdata);
  ret = cb(out, &bind, data);
  if (ret < 2)
  {
    return 0;
  }

  /* return length */
  vb[1] = 2 + oid_len + ret;
  return 2 + vb[1];
}

void
snmp_net_main(void)
{
  uint8_t *req = (uint8_t *) uip_appdata;
  int16_t len = uip_len;

  if (!uip_newdata())
  {
    return;
  }

  /* check sequence type and length */
  len -= 2;
  uint8_t *req_hdr = req;
  if (len < 0 || *(req++) != SNMP_TYPE_SEQUENCE)
  {
    return;
  }
  if (*(req++) != len)
  {
    return;
  }

  /* check version */
  len -= 3;
  if (len < 0 || *(req++) != SNMP_TYPE_INTEGER || *(req++) != 1 ||
      *(req++) != SNMP_VERSION1_VALUE)
  {
    return;
  }

  /* check community string */
  len -= 2;
  if (len < 0 || *(req++) != SNMP_TYPE_STRING)
  {
    return;
  }
  uint8_t cs_len = *(req++);
  if (cs_len > len)
  {
    return;
  }
  len -= cs_len;
  uint8_t *cs_ref = (uint8_t *) PSTR(SNMP_COMMUNITY_STRING);
  while (cs_len > 0)
  {
    if (pgm_read_byte(cs_ref++) != *(req++))
    {
      return;
    }
    cs_len--;
  }
  if (pgm_read_byte(cs_ref) != 0)
  {
    return;
  }

  /* check request type and length */
  len -= 2;
  uint8_t *req_type = req++;
  if (len < 0 ||
      (*req_type != SNMP_TYPE_GETREQ && *req_type != SNMP_TYPE_GETNEXTREQ))
  {
    return;
  }
  if (*(req++) != len)
  {
    return;
  }

  /* skip request id */
  len -= 2;
  if (len < 0 || *(req++) != SNMP_TYPE_INTEGER)
  {
    return;
  }
  uint8_t id_len = *(req++);
  if (id_len > len)
  {
    return;
  }
  req += id_len;
  len -= id_len;

  /* skip error */
  len -= 3;
  if (len < 0 || *(req++) != SNMP_TYPE_INTEGER || *(req++) != 1)
  {
    return;
  }
  uint8_t *err = req++;

  /* skip error index */
  len -= 3;
  if (len < 0 || *(req++) != SNMP_TYPE_INTEGER || *(req++) != 1)
  {
    return;
  }
  req++;

  /* get varbind list header */
  len -= 2;
  uint8_t *vb_list_hdr = req;
  if (len < 0 || *(req++) != SNMP_TYPE_SEQUENCE)
  {
    return;
  }
  uint8_t vb_list_len = *(req++);
  if (vb_list_len != len || vb_list_len > SNMP_MAX_OID_BUFFERSIZE)
  {
    return;
  }

  /* copy varbind list since we build the answer in place */
  uint8_t *vb_list = __builtin_alloca(vb_list_len);
  memcpy(vb_list, req, vb_list_len);

  /* process bindings */
  *err = SNMP_ERR_NONE;
  int8_t len_delta = 0;
  uint8_t bind_count = 0;
  while (len > 0)
  {
    /* check maximum bind count */
    if ((bind_count++) > SNMP_MAX_BIND_COUNT)
    {
      return;
    }

    /* check varbind */
    len -= 2;
    if (len < 0 || *(vb_list++) != SNMP_TYPE_SEQUENCE)
    {
      return;
    }
    uint8_t vb_len = *(vb_list++);
    if (vb_len > len)
    {
      return;
    }
    len -= vb_len;
    int16_t vb_rem = vb_len;

    /* check OID */
    vb_rem -= 2;
    if (vb_rem < 0 || *(vb_list++) != SNMP_TYPE_OID)
    {
      return;
    }
    uint8_t oid_len = *(vb_list++);
    if (oid_len > vb_rem)
    {
      return;
    }
    uint8_t *oid = vb_list;
    vb_list += oid_len;
    vb_rem -= oid_len;

    /* check null value */
    if (vb_rem != 2 || *(vb_list++) != SNMP_TYPE_NULL || *(vb_list++) != 0)
    {
      return;
    }

    /* process OID */
    uint8_t ret = snmp_proc_bind(*req_type, oid, oid_len, req);
    req += ret;
    if (ret == 0)
    {
      *err = SNMP_ERR_NO_SUCH_NAME;
    }

    /* update lengths */
    len_delta += (int8_t) (ret - vb_len - 2);
  }

  /* set response type */
  req_type[0] = SNMP_TYPE_GETRESP;

  /* update lengths */
  req_hdr[1] += len_delta;
  req_type[1] += len_delta;
  vb_list_hdr[1] += len_delta;

  struct uip_udpip_hdr *udpip_hdr =
    (struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN);

  uip_udp_conn_t conn;
  uip_ipaddr_copy(conn.ripaddr, udpip_hdr->srcipaddr);
  conn.rport = udpip_hdr->srcport;
  conn.lport = HTONS(SNMP_PORT);

  uip_udp_conn = &conn;

  /* Send immediately */
  uip_slen = uip_len + len_delta;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}

/*
  -- Ethersex META --
  header(protocols/snmp/snmp_net.h)
  net_init(snmp_net_init)
*/
