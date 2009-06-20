################################
# ecmd Global Variables
################################
define(`ECMD_USED', `ifdef(`ecmd_used', `', `dnl
define(`old_divert', divnum)dnl
define(`ecmd_used')dnl
divert(globals_divert)dnl
#define C6_ECMD_USED 1
#ifndef ECMD_PARSER_SUPPORT
#error Please define ECMD
#endif
')dnl
divert(old_divert)')


dnl
dnl  ECMD_GLOBAL(variable-name, initializer)
dnl
define(`ECMD_GLOBAL', `ECMD_USED()define(`old_divert', divnum)dnl
divert(globals_divert)dnl
const char PROGMEM $1_text[] = "$1";
divert(ecmd_variable_divert)dnl
{ $1_text, $2 },
`#define $1 (c6_ecmd_vars[' ecmd_global_count `].value)'
define(`ecmd_global_count', incr(ecmd_global_count))dnl
divert(old_divert)')

