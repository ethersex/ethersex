################################
# Conditionals
################################
define(`ON', `if (')
define(`UNLESS', `if (! ')
define(`DO', `ifelse(`$#', 0, `) {', `THREAD($1)divert(normal_divert)) { THREAD_START($1) }divert(action_divert)')')
define(`END', `ifelse(`$#', 0, `}', `THREAD_END($1)}')')
define(`BETWEEN', `$1 > $2 && $1 < $3')
define(`NOT', `ifelse(`$#', 0, `!', `! ( $1 )')')

################################
# Pins
################################
define(`PIN_INPUT', `define(`old_divert', divnum)divert(init_divert)  DDR_CONFIG_IN($1);
divert(old_divert)dnl')
define(`PIN_OUTPUT', `define(`old_divert', divnum)divert(init_divert)  DDR_CONFIG_OUT($1);
divert(old_divert)dnl')
define(`PIN_PULLUP', `define(`old_divert', divnum)divert(init_divert)  PIN_SET($1);
divert(old_divert)dnl')

define(`PIN_NEW', `ifdef(`pin_$1_used', `', `define(`pin_$1_used')define(`old_divert', divnum)dnl
divert(pin_table_divert)  { 0 },dnl
define(`pin_$1_idx', pin_count)dnl
define(`pin_count', incr(pin_count))dnl
divert(init_divert) pin_states[pin_$1_idx].old_state = PIN_HIGH($1) ? 1 : 0; 
divert(update_pin_divert)  
  pin_states[pin_$1_idx].old_state = PIN_HIGH($1) ? 1 : 0; 
divert(old_divert)')')

define(`PIN_RISING', `PIN_NEW($1)(!pin_states[pin_$1_idx].old_state && PIN_HIGH($1))')
define(`PIN_FALLING', `PIN_NEW($1)(pin_states[pin_$1_idx].old_state && !PIN_HIGH($1))')
define(`PIN_LOW', `!PIN_HIGH($1)')

divert(pin_table_divert)
struct pin_state {
  uint8_t old_state;
};

struct pin_state pin_states[] ={
divert(-1)

