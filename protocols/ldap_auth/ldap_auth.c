/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "ldap_auth.h"


#define STATE (&ldap_auth_conn->appstate.ldap_auth)

static uip_conn_t *ldap_auth_conn;

struct ldap_bind_request_t {
  uint8_t section[2]; /* type, length */
    uint8_t message_id[3]; /* type, length, id */
    uint8_t application[2]; /* type, length, id */
      uint8_t version[3]; /* type, length, version */
      uint8_t bind_dn[]; /* type, length, variable rest */
};

static void
ldap_auth_send_bind(void) {

  struct ldap_bind_request_t *bind = uip_sappdata;
  bind->section[0] = 0x30; /* type: section */

  bind->message_id[0] = 2; /* type: integer */
  bind->message_id[1] = 1; /* length: 1 */
  bind->message_id[2] = STATE->msgid; /* message id */

  bind->application[0] = 0x60; /* type: APPLICATION 0 */

  bind->version[0] = 2; /* type: integer */
  bind->version[1] = 1; /* length: 1 */
  bind->version[2] = 3; /* version 3 */

  bind->bind_dn[0] = 4; /* type: octet stream */
  bind->bind_dn[1] = sprintf(&bind->bind_dn[2], CONF_LDAP_AUTH_DN, STATE->username);

  char *ptr = &bind->bind_dn[0] + 2 + bind->bind_dn[1];
  ptr[0] = 0x80;
  ptr[1] = sprintf(&ptr[2], "%s", STATE->password);
  
  bind->application[1] = 3 + bind->bind_dn[1] + 2 
                      + ptr[1] + 2;
  bind->section[1] = bind->application[1] + 5;

  uip_send(uip_sappdata, 2 + bind->section[1]);
  LDAP_AUTH_DEBUG("bind request sent\n");
}

static void
ldap_auth_do(const char *username, const char *password)
{
  if (!ldap_auth_conn)
    ldap_auth_init();

  if (!ldap_auth_conn || STATE->pending) {
    // FIXME: another request on the wire
    LDAP_AUTH_DEBUG("auth error\n");
    return;
  }
  STATE->pending = 1;
  strncpy(STATE->username, username, sizeof(STATE->username));
  strncpy(STATE->password, password, sizeof(STATE->password));
}


static void
ldap_auth_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	LDAP_AUTH_DEBUG ("connection aborted\n");
	ldap_auth_conn = NULL;
    }

    if (uip_closed()) {
	LDAP_AUTH_DEBUG ("connection closed\n");
	ldap_auth_conn = NULL;
    }


    if (uip_connected()) {
	LDAP_AUTH_DEBUG ("new connection\n");
    }

    if ((uip_poll() || uip_rexmit()) && STATE->pending) {
      ldap_auth_send_bind();
    }

    if (uip_acked())
      STATE->msgid ++;

    if (uip_newdata() && uip_len) {
        STATE->pending = 0;
        struct ldap_bind_request_t *bind = uip_sappdata;
        LDAP_AUTH_DEBUG ("bind result: %x\n", bind->version[2]);
    }
}


void
ldap_auth_periodic(void)
{
    if (!ldap_auth_conn)
	ldap_auth_init();
}


void
ldap_auth_init(void)
{
    LDAP_AUTH_DEBUG ("initializing ldap auth connection\n");

    uip_ipaddr_t ip;
    set_CONF_LDAP_AUTH_IP(&ip);
    ldap_auth_conn = uip_connect(&ip, HTONS(389), ldap_auth_main);

    if (! ldap_auth_conn) {
	LDAP_AUTH_DEBUG ("no uip_conn available.\n");
	return;
    }

    STATE->pending = 0;
    STATE->msgid = 1;
    ldap_auth_do("test", "ethersex23");

}

/*
  -- Ethersex META --
  header(protocols/ldap_auth/ldap_auth.h)
  net_init(ldap_auth_init)
  timer(500, ldap_auth_periodic())
*/
