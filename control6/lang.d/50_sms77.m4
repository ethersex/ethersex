#########################################
# send sms via sms77
#########################################
define(`SMS77_USED', `ifdef(`sms77_used', `', `dnl
define(`old_divert', divnum)dnl
define(`sms77_used')dnl
divert(globals_divert)
#include "protocols/sms77/sms77.h"

#ifndef SMS77_SUPPORT
#error Please define sms77 support
#endif

divert(old_divert)')')

define(`SMS77', `SMS77_USED()ifelse(`$#', 1, `sms77_send($1);', `
    define(`__sms77_msg', `$1')dnl
    snprintf_P((char *)uip_buf, sizeof(uip_buf), PSTR($1), shift($@));
    sms77_send((char *)uip_buf);
')')')

