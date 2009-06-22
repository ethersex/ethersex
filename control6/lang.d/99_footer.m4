###############################
# Global flags
###############################
define(`STARTUP', `ifdef(`startup_used', `', `dnl
define(`old_divert', divnum)dnl
define(`startup_used')dnl
divert(globals_divert)static uint8_t global_started;
divert(control_end_divert)

  global_started = 1;dnl
divert(old_divert)')global_started == 0')

divert(0)dnl

