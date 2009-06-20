################################
# CLOCK
################################
define(`CLOCK_USED', `ifdef(`clock_used', `', `dnl
define(`clock_used')dnl
`define(`old_divert', divnum)dnl
divert(globals_divert)struct clock_datetime_t datetime;
uint8_t last_minute;
#ifndef CLOCK_SUPPORT
#error Please define clock support
#endif

divert(normal_start_divert)  clock_localtime(&datetime, clock_get_time());
divert(control_end_divert)  last_minute = datetime.min;
divert(old_divert)'')')

define(`CLOCK_SEC', `CLOCK_USED()datetime.sec')
define(`CLOCK_MIN', `CLOCK_USED()(datetime.min != last_minute) && datetime.min')
define(`CLOCK_HOUR', `CLOCK_USED()datetime.hour')
define(`CLOCK_DAY', `CLOCK_USED()datetime.day')
define(`CLOCK_MONTH', `CLOCK_USED()datetime.month')
define(`CLOCK_DOW', `CLOCK_USED()datetime.dow')
define(`CLOCK_YEAR', `CLOCK_USED()datetime.year')

