################################
# RC5
################################
define(`RC5_USED', `ifdef(`rc5_used', `', `dnl
define(`old_divert', divnum)dnl
define(`rc5_used')dnl
divert(globals_divert)
#ifndef RC5_SUPPORT
#error Please define rc5 support
#endif

#include "hardware/ir/rc5/rc5.h"

static uint16_t
control6_get_rc5(void){
  if (rc5_global.len == 0) return 0;
  uint16_t ret = 
    (rc5_global.queue[rc5_global.len - 1].address * 256)
     | rc5_global.queue[rc5_global.len - 1].code;
  rc5_global.len --;
  return ret;
}

divert(old_divert)')')

define(`RC5_GET', `RC5_USED()control6_get_rc5()')

