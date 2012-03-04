#########################################
# send strings via http
#########################################
define(`HTTPLOG_USED', `ifdef(`httplog', `', `dnl
define(`old_divert', divnum)dnl
define(`httplog_used')dnl
divert(globals_divert)
#include "protocols/httplog/httplog.h"

#ifndef HTTPLOG_SUPPORT
#error Please define httplog support
#endif

divert(old_divert)')')

define(`HTTPLOG', `HTTPLOG_USED()ifelse(`$#', 1, `httplog_P($1)', `
    httplog_P(PSTR($1), shift($@));
')')')

