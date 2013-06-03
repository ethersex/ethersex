dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

divert(0)dnl
#define TCP_STATE (&uip_conn->appstate.control6_tcp)

divert(-1)

dnl ==========================================================================
dnl TCP_EXPECT(text)
dnl ==========================================================================
define(`TCP_EXPECT', `
  PT_WAIT_UNTIL(pt, uip_len && uip_newdata ());
  if (strstr_P (uip_appdata, PSTR($1)) == NULL) {
    uip_close ();
    PT_EXIT(pt);
  }
')


dnl ==========================================================================
dnl TCP_SEND(text)
dnl ==========================================================================
define(`TCP_SEND', `dnl
  ifelse(`$#', 1,dnl
    `strcpy_P(uip_sappdata, PSTR($1));
     uip_send (uip_sappdata, strlen (uip_sappdata));',

    dnl multiple args, use printf syntax
    `{ uint16_t i = snprintf_P (uip_sappdata, uip_mss (), PSTR($1), shift($@));
       uip_send (uip_sappdata, i ); }')
  PT_YIELD(pt);
')

dnl ==========================================================================
dnl TCP_HANDLER_PERSIST(name)
dnl ==========================================================================
define(`TCP_HANDLER_PERSIST', `dnl
divert(action_divert)dnl
static void
c6_tcp_handler_$1 (void)
{
  if (uip_aborted () || uip_timedout () || uip_closed ()) {
    /* connection was reset, reconnect. */
    uip_connect (&uip_conn->ripaddr, uip_conn->rport, c6_tcp_handler_$1);
    return;
  }

  PT_THREAD(inline_thread(struct pt *pt)) {
    PT_BEGIN(pt);
')

dnl ==========================================================================
dnl TCP_HANDLER(name)
dnl ==========================================================================
define(`TCP_HANDLER', `dnl
divert(action_divert)dnl
static void
c6_tcp_handler_$1 (void)
{
  if (uip_aborted () || uip_timedout () || uip_closed ())
    return;

  PT_THREAD(inline_thread(struct pt *pt)) {
    PT_BEGIN(pt);
')

dnl ==========================================================================
dnl TCP_HANDLER_END()
dnl ==========================================================================
define(`TCP_HANDLER_END', `
    uip_close();
    PT_END(pt);
  }

  if (uip_newdata ())
    ((char *) uip_appdata)[uip_len] = 0;

  if (uip_rexmit ())
    /* restore old lc context to automatically do the retransmit */
    TCP_STATE->pt.lc = TCP_STATE->rexmit_lc;

  inline_thread(&TCP_STATE->pt);
  TCP_STATE->rexmit_lc = TCP_STATE->pt.lc;
}
divert(normal_divert)')



dnl ==========================================================================
dnl TCP_CONNECT(ip, port, handler)
dnl ==========================================================================
define(`TCP_CONNECT', `do {
  IPADDR($1);
  /* uip_conn_t *conn = */ uip_connect (&ip, HTONS ($2), c6_tcp_handler_$3);
} while(0)
')



dnl ==========================================================================
dnl TCP_LISTEN(port, handler)
dnl ==========================================================================
define(`TCP_LISTEN', `do {
  uip_listen(HTONS($1), c6_tcp_handler_$2);
} while(0)
')

