################################
# RC5
################################
define(`PS2_USED', `ifdef(`ps2_used', `', `dnl
define(`old_divert', divnum)dnl
define(`ps2_used')dnl
divert(globals_divert)
#ifndef PS2_SUPPORT
#error Please define ps2 support
#endif

#include "hardware/input/ps2/ps2.h"

static struct key_press 
control6_get_ps2(void){
  struct key_press tmp, *ptr = &ps2_key_cache[0];
  while(ptr[1].keycode) ptr++;
  tmp = *ptr;
  memset(ptr, 0, sizeof(*ps2_key_cache)); 
  return tmp;
}

divert(old_divert)')')

define(`PS2_GET', `PS2_USED()control6_get_ps2()')

