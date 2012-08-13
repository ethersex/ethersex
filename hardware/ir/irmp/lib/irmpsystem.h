/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmpsystem.h - system specific includes and defines
 *
 * Copyright (c) 2009-2012 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmpsystem.h,v 1.6 2012/05/23 14:02:45 fm Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _IRMPSYSTEM_H_
#define _IRMPSYSTEM_H_

#if !defined(_IRMP_H_) && !defined(_IRSND_H_)
#  error please include only irmp.h or irsnd.h, not irmpsystem.h
#endif

#if defined(__18CXX)                                                                // Microchip PIC C18 compiler
#  define PIC_C18
#elif defined(__PCM__) || defined(__PCB__) || defined(__PCH__)                      // CCS PIC compiler
#  define PIC_CCS
#elif defined(STM32L1XX_MD) || defined(STM32L1XX_MDP) || defined(STM32L1XX_HD)      // ARM STM32
#  include <stm32l1xx.h>
#  define ARM_STM32
#  define ARM_STM32L1XX
#elif defined(STM32F10X_LD) || defined(STM32F10X_LD_VL) \
   || defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) \
   || defined(STM32F10X_HD) || defined(STM32F10X_HD_VL) \
   || defined(STM32F10X_XL) || defined(STM32F10X_CL)                                // ARM STM32
#  include <stm32f10x.h>
#  define ARM_STM32
#  define ARM_STM32F10X
#elif defined(STM32F4XX)                                                            // ARM STM32
#  include <stm32f4xx.h>
#  define ARM_STM32
#  define ARM_STM32F4XX
#elif defined(unix) || defined(WIN32)                                               // Unix/Linux or Windows
#  define UNIX_OR_WINDOWS
#else
#  define ATMEL_AVR                                                                 // ATMEL AVR
#endif

#include <string.h>

#ifdef UNIX_OR_WINDOWS                                                              // Analyze on Unix/Linux or Windows
#  include <stdio.h>
#  include <stdlib.h>
#  define F_CPU 8000000L
#  define ANALYZE
#  define DEBUG
#  ifdef unix
#    include <stdint.h>
#  else
typedef unsigned char                   uint8_t;
typedef unsigned short                  uint16_t;
#  endif
#endif


#if defined(ATMEL_AVR)
#  include <stdint.h>
#  include <stdio.h>
#  include <avr/io.h>
#  include <util/delay.h>
#  include <avr/pgmspace.h>
#  include <avr/interrupt.h>
#  define IRSND_OC2                     0       // OC2
#  define IRSND_OC2A                    1       // OC2A
#  define IRSND_OC2B                    2       // OC2B
#  define IRSND_OC0                     3       // OC0
#  define IRSND_OC0A                    4       // OC0A
#  define IRSND_OC0B                    5       // OC0B
#else
#  define PROGMEM
#  define memcpy_P                      memcpy
#endif

#if defined(PIC_CCS) || defined(PIC_C18) || defined(ARM_STM32)
typedef unsigned char                   uint8_t;
typedef unsigned short                  uint16_t;
#endif

#if defined (PIC_C18)
#  include <p18cxxx.h>                                                              // main PIC18 h file
#  include <timers.h>                                                               // timer lib
#  include <pwm.h>                                                                  // pwm lib
#  define IRSND_PIC_CCP1                1                                           // PIC C18 RC2 = PWM1 module
#  define IRSND_PIC_CCP2                2                                           // PIC C18 RC1 = PWM2 module
#endif

#ifndef TRUE
#  define TRUE                          1
#  define FALSE                         0
#endif

#ifndef IRMP_USE_AS_LIB
typedef struct
{
  uint8_t                               protocol;                                   // protocol, i.e. NEC_PROTOCOL
  uint16_t                              address;                                    // address
  uint16_t                              command;                                    // command
  uint8_t                               flags;                                      // flags, e.g. repetition
} IRMP_DATA;
#endif

#endif // _IRMPSYSTEM_H_
