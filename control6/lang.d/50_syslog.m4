#########################################
# send Syslog messages
#########################################
define(`SYSLOG_USED', `ifdef(`syslog_used', `', `dnl
define(`old_divert', divnum)dnl
define(`syslog_used')dnl
divert(globals_divert)
#include "protocols/syslog/syslog.h"

divert(old_divert)')')

define(`REQUIRE_SYSLOG', `SYSLOG_USED()
#ifndef SYSLOG_SUPPORT
#error Please define syslog support
#endif
')

define(`SYSLOG', `SYSLOG_USED()#ifdef SYSLOG_SUPPORT
ifelse(`$#', 1, `syslog_sendf_P(PSTR($1))', `syslog_sendf_P(PSTR($1), shift($*))');
#endif  /* SYSLOG_SUPPORT */
')

