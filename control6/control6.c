#include <avr/io.h>
#include "../clock/clock.h"
#include "../pt/pt.h"
#include "../config.h"

struct action {
  uint8_t started;
  struct pt pt;
};



struct action action_threads[] = {
 {0, {0} },
};

uint32_t timers[] = {
 -1,
};

uint32_t act_time;

struct pin_state {
  uint8_t old_state;
};

struct pin_state pin_states[] ={
  { 0 },
};


static 
PT_THREAD(action_thread_new(struct pt *pt)) {
  PT_BEGIN(pt);

      
  timers[0] = clock_get_time();
      PIN_SET(LED);
      PT_WAIT_UNTIL(pt, (act_time - timers[0]) >= 20);
      PIN_CLEAR(LED);
  PT_WAIT_WHILE(pt, 1);
  PT_END(pt);
}

void control6_init(void) {
  DDR_CONFIG_IN(TASTER);
  PIN_SET(TASTER);
  DDR_CONFIG_OUT(LED);
 pin_states[0].old_state = PIN_HIGH(TASTER) ? 1 : 0; 
}

void control6_run(void) {
  
      
  act_time = clock_get_time();
  if ( (pin_states[0].old_state && !PIN_HIGH(TASTER)) ) { do { action_threads[0].started = 1; 
  PT_INIT(&action_threads[0].pt); } while(0); }
  


  if (action_threads[0].started) { action_thread_new(&action_threads[0].pt); }  
  pin_states[0].old_state = PIN_HIGH(TASTER) ? 1 : 0; 

}
