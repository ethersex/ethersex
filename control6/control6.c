#include <avr/io.h>
#include "services/clock/clock.h"
#include "pt/pt.h"
#include "config.h"
#include "kty/kty81.h"
#ifdef NAMED_PIN_SUPPORT
#include "named_pin/user_config.h"
#endif
#include "net/ecmd_sender_net.h"
#include "core/bit-macros.h"
#include "uip/uip.h"
#include "control6.h"

struct action {
  uint8_t started;
  struct pt pt;
};





enum sensor { Kessel,Vorlauf,Wasser,WasserRuecklauf,WasserZulauf,AussenNord,Ruecklauf,Ofen };



struct action action_threads[] = {

};

uint32_t timers[] = {

};


struct pin_state {
  uint8_t old_state;
};

struct pin_state pin_states[] ={

};

#ifdef C6_ECMD_USED
struct c6_option_t {
  PGM_P name;
  uint8_t value;
};

struct c6_option_t c6_ecmd_vars[] = {
  /* hier alle variablen definieren */
};
uint8_t control6_set(const char *varname, uint8_t value) {
  uint8_t i;
  for(i = 0;i <  0 ;i ++) {
    if (strcmp_P(varname, c6_ecmd_vars[i].name) == 0) {
      c6_ecmd_vars[i].value = value;
      return 1;
    }
  }
  return 0;
}
uint8_t control6_get(const char *varname, uint8_t *value) {
  uint8_t i;
  for(i = 0;i <  0 ;i ++) {
    if (strcmp_P(varname, c6_ecmd_vars[i].name) == 0) {
      *value = c6_ecmd_vars[i].value;
      return 1;
    }
  }
  return 0;
}

#endif  /* C6_ECMD_USED */
void control6_init(void) {
}

void control6_run(void) {











}
