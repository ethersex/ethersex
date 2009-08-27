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
#include "pam_ldap.h"
#include "pam_prototypes.h"


#define STATE (&ldap_auth_conn->appstate.ldap_auth)

static uip_conn_t *ldap_auth_conn;

struct ldap_bind_request_t {
  uint8_t section[2]; /* type, length */
    uint8_t message_id[3]; /* type, length, id */
    uint8_t application[2]; /* type, length, id */
      uint8_t version[3]; /* type, length, version */
      uint8_t bind_dn[]; /* type, length, variable rest */
};

struct pam_cache_entry {
  char username[16];
  char password[16];
  uint8_t timeout;
};


#define CACHE_SIZE 3
#define CACHE_TIME 6 /* expands to ca 10 seconds */
struct pam_cache_entry cache[CACHE_SIZE];


static void
pam_ldap_send_bind(void) {

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
  bind->bind_dn[1] = sprintf_P((char*)&bind->bind_dn[2], PSTR(CONF_LDAP_AUTH_DN),
                             STATE->username);

  uint8_t *ptr = &bind->bind_dn[0] + 2 + bind->bind_dn[1];
  ptr[0] = 0x80;
  ptr[1] = sprintf((char *)&ptr[2], "%s", STATE->password);
  
  bind->application[1] = 3 + bind->bind_dn[1] + 2 
                      + ptr[1] + 2;
  bind->section[1] = bind->application[1] + 5;

  uip_send(uip_sappdata, 2 + bind->section[1]);
  LDAP_AUTH_DEBUG("bind request sent %s:%s\n", STATE->username,
                  STATE->password);
}

static void
pam_ldap_main(void)
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
      pam_ldap_send_bind();
    }

    if (uip_acked())
      STATE->msgid ++;

    if (uip_newdata() && uip_len) {
        struct ldap_bind_request_t *bind = uip_appdata;
        STATE->pending = 0;
        // FIXME here set the auth result
        LDAP_AUTH_DEBUG ("bind result: %x\n", bind->version[2]);
        if (bind->version[2] == 0) {
          *STATE->auth_state = PAM_SUCCESS;
          uint8_t i, oldest_entry = 0, min = 255;
          for (i = 0; i < CACHE_SIZE; i++) {
            if (cache[i].timeout < min) {
              min = cache[i].timeout;
              oldest_entry = i;
            }
          }
          strncpy(cache[oldest_entry].username, STATE->username, 
                  sizeof(cache[0].username));
          strncpy(cache[oldest_entry].password, STATE->password, 
                  sizeof(cache[0].password));
          cache[oldest_entry].timeout = CACHE_TIME;
          
        }
        else
          *STATE->auth_state = PAM_DENIED;
        STATE->auth_state = NULL;
    }
}


void
pam_ldap_periodic(void)
{
    if (!ldap_auth_conn)
	pam_ldap_init();

    uint8_t i;
    for (i = 0; i < CACHE_SIZE; i++) 
      if (cache[i].timeout) cache[i].timeout --;

}


void
pam_ldap_init(void)
{
    LDAP_AUTH_DEBUG ("initializing ldap auth connection\n");

    uip_ipaddr_t ip;
    set_CONF_LDAP_AUTH_IP(&ip);
    ldap_auth_conn = uip_connect(&ip, HTONS(389), pam_ldap_main);

    if (! ldap_auth_conn) {
	LDAP_AUTH_DEBUG ("no uip_conn available.\n");
	return;
    }

    STATE->pending = 0;
    STATE->msgid = 1;
    //ldap_auth_do("test", "ethersex23");

}

void
pam_auth(char *username, char *password, uint8_t *auth_state)
{
  if (!ldap_auth_conn)
    pam_ldap_init();

  /* Check the authentification cache */
  uint8_t i;
  for (i = 0; i < CACHE_SIZE; i++)  
    if ((  strncmp(cache[i].username, username, sizeof(cache[0].username)) == 0)
        &&(strncmp(cache[i].password, password, sizeof(cache[0].password)) == 0)
        && cache[i].timeout != 0) {
      *auth_state = PAM_SUCCESS;
      LDAP_AUTH_DEBUG("auth success through caching\n");
      return;
    }
    

  if (!ldap_auth_conn || STATE->pending) {
    // FIXME: another request on the wire
    LDAP_AUTH_DEBUG("auth error\n");
    *auth_state = PAM_DENIED;
    return;
  }
  *auth_state = PAM_PENDING;
  STATE->auth_state = auth_state;
  STATE->pending = 1;
  strncpy(STATE->username, username, sizeof(STATE->username));
  strncpy(STATE->password, password, sizeof(STATE->password));
}



/*
  -- Ethersex META --
  header(services/pam/pam_ldap.h)
  net_init(pam_ldap_init)
  timer(250, pam_ldap_periodic())

  state_header(services/pam/pam_ldap_state.h)
  state_tcp(struct ldap_auth_connection_state_t ldap_auth)
*/
