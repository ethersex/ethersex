/*
   ATmega328p is very similar to ATmega168p, therefore we just include
   that one's hackery internals first.
 */

include(internals/hackery_atmega168p.m4)

#define _ATMEGA328P
#undef _ATMEGA168P
