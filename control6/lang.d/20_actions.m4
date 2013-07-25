################################
# Actions
################################
define(`THREAD', `define(`action_thread_ident', __line__)divert(0)dnl
 {0, {0} },define(`action_thread_$1_idx', action_thread_count)dnl
define(`action_thread_count', incr(action_thread_count))dnl
divert(action_divert)dnl

/* Thread: action_thread_ident */

static 
PT_THREAD(action_thread_$1(struct pt *pt)) {
  PT_BEGIN(pt);
divert(normal_end_divert)
  if (action_threads[action_thread_$1_idx].started) { THREAD_DO($1) }dnl
divert(action_divert)')

define(`INTHREAD', `ifdef(`action_thread_ident', `$1', `$2')')
define(`DIE', `errprint(`ERROR: $1
')m4exit(255)')

define(`THREAD_END', `undefine(`action_thread_ident')divert(action_divert)dnl
dnl PT_WAIT_WHILE(pt, 1);
  PT_RESTART(pt);
  PT_END(pt);		/* mmh, not really nice, since not reached.
			   I hope the compiler is intelligent enough *g*  */
}

divert(normal_divert)')
define(`THREAD_DO', `dnl
action_threads[action_thread_$1_idx].started =
  (PT_SCHEDULE (action_thread_$1(&action_threads[action_thread_$1_idx].pt)));')

define(`THREAD_START',  `action_threads[action_thread_$1_idx].started = 1;')
define(`THREAD_STOP',  `action_threads[action_thread_$1_idx].started = 0;')
define(`THREAD_WAIT',  `action_threads[action_thread_$1_idx].started = 0;
PT_WAIT_WHILE(pt, action_threads[action_thread_$1_idx].started == 1);')
define(`THREAD_RESTART',  `do { action_threads[action_thread_$1_idx].started = 1; 
  PT_INIT(&action_threads[action_thread_$1_idx].pt); } while(0);')
divert(action_table_divert)dnl
struct action {
  uint8_t started;
  struct pt pt;
};

define(`THREAD_EXIT', `PT_EXIT (pt);')

define(`THREAD_STARTED', `action_threads[action_thread_$1_idx].started')

divert(-1)
