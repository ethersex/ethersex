e6 scheduler module
===================

Basics
------
The e6 scheduler module controls the periodic timer and millitimer invocations
using the e6 meta framework.

[(ref. e6 Wiki / Concept of meta)](http://www.ethersex.de/index.php/Concept_of_meta).

The scheduler supports periodic timers with an interval down to one millisecond
(if the configured periodic tick frequency is at least 1 kHz) using the new-style
`millitimer` macro. The `millitimer` interval is specified in milliseconds.
If the interval specified can not be achieved with the config settings of the
periodic module, the user will be prompted with an error during compilation.

The scheduler supports also (old-fashioned) `timer(n, ...)` macros for compatibility
reasons. The timer interval is n * 1/HZ, i.e. n * 20 ms. `timer` macros will be
converted to new-style `millitimer` macros by `scripts/meta_magic_scheduler.m4`.

### Static and dynamic timers ###

Timers may be either *static* or *dynamic*.

Static timers are declared in the e6 meta section of a C implementation file
and may not be modified during run-time:

    /*
      -- Ethersex META --
      header(core/scheduler/scheduler_test.h)
      /* a timer called every second using the old-style timer macro */
      ifdef(`conf_DEBUG_SCHEDULER',`timer(50, scheduler_test_periodic_t50)')
      /* a timer called every second using the millitimer macro */
      ifdef(`conf_DEBUG_SCHEDULER',`millitimer(1000, scheduler_test_periodic_mt1000)')
    */

Dynamic timers may be added during run-time using the dynamic timer API. Dynamic
timers may be added, deleted, suspended and resumed or restarted and their interval
may be changed.

The dynamic timer API additionally offers *one-shot timers*. One-shot timers are
automatically deleted after invocation and are a very nice "e6-multi-tasking"-friendly
option to generate delays *without* blocking program execution (contrary to the the
infamous delay_* macros).

Have a look at core/scheduler/dynamic.h for the API documentation.

### Timer distribution ###

The meta framework for the scheduler tries to equally distribute all static timers
in a given number of time slots of 1.5 times the count of periodic ticks per second,
i.e. in 1.5 s, to distribute the MCU-load.

To achieve this the meta framework of the scheduler modifies the *initial* value
of a static timers delay counter normally counting down periodically from the
interval specified to zero with a value slightly higher than the interval specified.

For example one uses three `millitimer(10, ...)` functions defined in different
modules and a configured periodic tick frequency of 1000 Hz. The meta framework
will initialize the delay counter of the first timer to 10, the second to 11
and the third to 12. The time of the first invocation after reset for the second
and third timer will be slightly delayed by 1 resp. 2 ms but after that the delay
counter will be reloaded with the interval specified and all timers will be called
every 10 ms but with a (at least planned) 1 ms time distance.

This mechanism is limited by the lack of information about the timers real run-time
behaviour. If a timer requires two or more periodic ticks to complete the mechanism
is effectively undermined.

Scheduler Configuration
-----------------------

The control block for a dynamic timer requires 7 Byte of precious RAM. To support
AVR-MCUs with smaller amounts of RAM, the maximum number of dynamic timers may be
configured using menuconfig or dynamic timers may be completely disabled.

(At least for now: dynamic timer auto-configuration via meta is on the TODO list)

Note that 4 byte - the function pointer and the constant interval - of the
required control information for *static* timers are moved to FLASH to save RAM.

Debugging / Testing
-------------------

WIP: See core/scheduler/scheduler_test.{c,h}

-----

Copyright (c) 2013-2015 by Michael Brakemeier <michael@brakemeier.de>