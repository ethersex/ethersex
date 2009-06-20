################################
# KTY81
################################
define(`KTY_USED', `ifdef(`kty_used', `', `dnl
define(`old_divert', divnum)dnl
define(`kty_used')dnl
divert(globals_divert)
#ifndef KTY_SUPPORT
#error Please define kty support
#endif

divert(old_divert)')')

define(`KTY_GET', `KTY_USED()temperatur(get_kty($1))')

