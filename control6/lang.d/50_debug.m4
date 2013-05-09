#########################################
# send debug messages
#########################################
divert(0)#include "core/debug.h"
divert(-1)define(`DEBUG', `debug_printf("Control6: " $1 "\n", shift($@));')
