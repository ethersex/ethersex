################################
# Global Variables
################################

define(`GLOBAL', `define(`old_divert', divnum)dnl
divert(globals_divert)ifelse(`$#', 2, `$2', `uint8_t') $1;
divert(old_divert)')

