################################
# Global Variables
################################

define(`GLOBAL', `define(`old_divert', divnum)dnl
divert(globals_divert)ifelse(`$#', 2, `$2', `uint8') $1;
divert(old_divert)')

