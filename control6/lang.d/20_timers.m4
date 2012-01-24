################################
# Timers
################################
define(`TIMER_NEW', `ifdef(`timer_$1', `', `dnl
define(`old_divert', divnum)
divert(timer_divert) -1,dnl
divert(old_divert)dnl
define(`timer_$1', timer_count)dnl
define(`timer_count', incr(timer_count))')')
define(`TIMER_START', `TIMER_NEW($1)  timers[timer_$1] = clock_get_uptime();')

define(`TIMER', `ifdef(`timer_used', `', `dnl
define(`old_divert', divnum)dnl
divert(globals_divert)uint32_t act_time;
#ifndef CLOCK_SUPPORT
#error Please define clock support
#endif

divert(normal_start_divert)act_time = clock_get_uptime();
define(`timer_used')dnl
divert(old_divert)')dnl
(act_time - timers[timer_$1])')

define(`TIMER_WAIT', `PT_WAIT_UNTIL(pt, TIMER($1) >= $2);')
define(`WAIT', `TIMER_START(`timer_on_'action_thread_ident); TIMER_WAIT(`timer_on_'action_thread_ident, ($1));')

