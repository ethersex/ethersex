/*
   AT90CAN32 is very similar to AT90CAN128, therefore we just include
   that one's hackery internals first.
 */

include(internals/hackery_at90can128.m4)

#define _AT90CAN32
#undef _AT90CAN128
