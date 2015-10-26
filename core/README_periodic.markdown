e6 periodic module
==================

Basics
------
The periodic module is responsible for maintaining e6 regular heartbeat.

Periodic controls (one of) the 16 bit timer/counter units of the AVR MCU.
The timer is initialized to emit regular ticks, the periodic milliticks,
with a configurable interval. These ticks are used to make the e6 heart
beating with more or less 50 Hz `(HZ)` and to generate the interval for
metas timer calls
[(ref. Concept of meta)](http://www.ethersex.de/index.php/Concept_of_meta).

Beside metas `timer` m4-macro, the periodic module offers an interface to
the periodic milliticks. Other modules may hook directly into the timer
ISR by using the `periodic_milliticks_isr` m4-macro to make use of the
configurable (possibly much higher than HZ) periodic milliticks frequency.

Timer Configuration
-------------------
The MCUs 16 bit Timer/Counter 1 (or 3 if available) will be configured
for Fast PWM mode, counting up from 0 to TOP. TOP is set via the Output
Compare Register (OCRnA). Overflow interrupts (TOV) will be used to
generate periodic milliticks. Using Fast PWM and OCRnA ensures double
buffered access and thus clean periodic interval adjustments when writing
updated TOP values.

Periodic timer constants are calculated from *Periodic ticks per second*
configured via menuconfig. Aim at exactness of the periodic milliticks
frequency and calculate its period time (i.e. PWM TOP-value) first and
then deduce all other timings from this one.

The value of *Periodic ticks per second* `(CONF_MTICKS_PER_SEC)` is
guaranteed to be a multiple of 50 Hz `(n * HZ)`, enforced by menuconfig.

Formula for Fast PWM frequency according to AVR datasheet:

    CONF_MTICKS_PER_SEC = F_CPU / (PERIODIC_PRESCALER * (1 + PERIODIC_TOP))
    => PERIODIC_TOP = (F_CPU / (PERIODIC_PRESCALER * CONF_MTICKS_PER_SEC)) - 1

Calculate required prescaler value, aim for highest timer/counter
resolution, i.e. lowest possible prescaler:

    PERIODIC_PRESCALER = F_CPU / (CONF_MTICKS_PER_SEC * (1 + MAX_OVERFLOW))

Debugging / Testing
-------------------

To aid hardware debugging and testing pins may be toggled at half the
periodic millitick and HZ frequencies. This might be useful to verify
and adjust the frequencies or to investigate some missing milliticks.

Hardware debugging and testing support may be enabled via the menuconfig
option *Toggle pins in periodic tick*. As a reference signal toggling the
hardware output compare pin at *Periodic ticks per second* / 2 may be enabled
via the option *Enable toggling of Timer/Counter OC1A-Pin* if Timer/Counter
1 is used. Use this option with care - the OC1A output pin is fixed. Do not
enable this unless the dedicated pin might be safely configured as output
on your board.

Additionally the pins have to be defined in your hardware user configuration
(in pinning/hardware/user_MYFABULOUSBOARD.m4).

Example for the Pollin AVR Net-IO board:

    dnl use ADC1 / ADC2 screw terminal
    ifdef(`conf_DEBUG_PERIODIC_WAVEFORMS', `dnl
      pin(PERIODIC_WAVE25HZ_OUT, PA4, OUTPUT)
      pin(PERIODIC_WAVETICK_OUT, PA5, OUTPUT)
    ')dnl
    
    dnl OC1A aka PD5 can be found on the Ext connector pin 4
    ifdef(`conf_DEBUG_PERIODIC_OC1A', `dnl
      pin(PERIODIC_OC1A_OUT, PD5, OUTPUT)dnl
    ')dnl


