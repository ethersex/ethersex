################################
# CLOCK
################################
define(`CLOCK_USED', `')dnl
define(`old_divert',divnum)dnl
divert(globals_divert)#ifdef C6_CLOCK_USED
clock_datetime_t datetime;
uint8_t last_minute;
#ifndef CLOCK_SUPPORT
#error Please define clock support
#endif
#if defined(CLOCK_DATETIME_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)
#else
#error Please define clock date/time support
#endif
#endif
divert(normal_divert)#ifdef C6_CLOCK_USED
clock_localtime(&datetime, clock_get_time());
#endif
divert(control_end_divert)#ifdef C6_CLOCK_USED
  last_minute = datetime.min;
#endif
divert(old_divert)

dnl PLEASE edit control6-header.m4 if you add something here
define(`CLOCK_SEC', `datetime.sec')
define(`CLOCK_MIN', `datetime.min')
define(`CLOCK_HOUR', `datetime.hour')
define(`CLOCK_DAY', `datetime.day')
define(`CLOCK_MONTH', `datetime.month')
define(`CLOCK_DOW', `datetime.dow')
define(`CLOCK_YEAR', `(datetime.year + 1900)')

define(`ONCE', `(datetime.min != last_minute) && ')
