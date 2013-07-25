#########################################
# send Twitter messages
#########################################
define(`TWITTER_USED', `ifdef(`twitter_used', `', `dnl
define(`old_divert', divnum)dnl
define(`twitter_used')dnl
divert(globals_divert)
#include "protocols/twitter/twitter.h"

#ifndef TWITTER_SUPPORT
#error Please define twitter support
#endif

divert(old_divert)')')

define(`TWITTER', `TWITTER_USED()ifelse(`$#', 1, `twitter_send($1)', `
    define(`__twitter_msg', `$1')dnl
    snprintf_P((char *)uip_buf, sizeof(uip_buf), PSTR($1), shift($@));
    twitter_send((char *)uip_buf);
')')')

