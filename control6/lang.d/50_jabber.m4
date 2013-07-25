#########################################
# send Jabber messages
#########################################
define(`JABBER_USED', `ifdef(`jabber_used', `', `dnl
define(`old_divert', divnum)dnl
define(`jabber_used')dnl
divert(globals_divert)
#include "services/jabber/jabber.h"

#ifndef JABBER_SUPPORT
#error Please define jabber support
#endif

divert(old_divert)')')

define(`JABBER', `JABBER_USED()ifelse(`$#', 1, `jabber_send_message($1)', `
    define(`__jabber_msg', `$1')dnl
    snprintf_P((char *)uip_buf, sizeof(uip_buf), PSTR($1), shift($@));
    jabber_send_message((char *)uip_buf);
')')')

