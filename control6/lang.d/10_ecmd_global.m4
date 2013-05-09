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

#include "control6/ecmd.c"

/*
  -- Ethersex META --
    ecmd_feature(c6_get, "c6 get ", VARNAME, Display the current value of the Control6 ECMD variable)
    ecmd_feature(c6_set, "c6 set ", VARNAME VALUE, Set an new value on the Control6 ECMD variable)
*/

')dnl
divert(old_divert)')


dnl
dnl  ECMD_GLOBAL(variable-name, initializer)
dnl
define(`ECMD_GLOBAL', `ECMD_USED()define(`old_divert', divnum)dnl
divert(globals_divert)dnl
const char PROGMEM $1_text[] = "$1";
divert(ecmd_variable_divert)dnl
{ .name = $1_text, .value = { .type = C6_TYPE_$3, .data.d_$3 = ifelse(`$2', `', `0', `$2') } },
`#define $1 (c6_ecmd_vars[' ecmd_global_count `].value.data.d_$3)'
define(`ecmd_global_count', incr(ecmd_global_count))dnl
divert(old_divert)')

