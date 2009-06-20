#########################################
# send Syslog messages
#########################################
define(`SYSLOG_USED', `ifdef(`syslog_used', `', `dnl
define(`old_divert', divnum)dnl
define(`syslog_used')dnl
divert(globals_divert)
#include "protocols/syslog/syslog.h"

#ifndef SYSLOG_SUPPORT
#error Please define syslog support
#endif

divert(old_divert)')')

define(`SYSLOG', `SYSLOG_USED()ifelse(`$#', 1, `syslog_send_P(PSTR($1))', `syslog_sendf($*)')')

