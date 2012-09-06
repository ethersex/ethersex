/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmp.h
 *
 * Copyright (c) 2009-2012 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmp.h,v 1.79 2012/05/23 12:26:25 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _IRMP_H_
#define _IRMP_H_

#include "irmpsystem.h"

#ifndef IRMP_USE_AS_LIB
#  include "irmpconfig.h"
#endif

#ifndef IRMP_USE_AS_LIB
#if defined (ATMEL_AVR)
#  define _CONCAT(a,b)                          a##b
#  define CONCAT(a,b)                           _CONCAT(a,b)
#  define IRMP_PORT                             CONCAT(PORT, IRMP_PORT_LETTER)
#  define IRMP_DDR                              CONCAT(DDR, IRMP_PORT_LETTER)
#  define IRMP_PIN                              CONCAT(PIN, IRMP_PORT_LETTER)
#  define IRMP_BIT                              IRMP_BIT_NUMBER
#  define input(x)                              ((x) & (1 << IRMP_BIT))
#elif defined (PIC_C18)
#  define input(x)                              (x)
#elif defined (PIC_CCS)
#  define input(x)                              (x)
#elif defined (ARM_STM32)
#  define _CONCAT(a,b)                          a##b
#  define CONCAT(a,b)                           _CONCAT(a,b)
#  define IRMP_PORT                             CONCAT(GPIO, IRMP_PORT_LETTER)
#  if defined (ARM_STM32L1XX)
#    define IRMP_PORT_RCC                       CONCAT(RCC_AHBPeriph_GPIO, IRMP_PORT_LETTER)
#  elif defined (ARM_STM32F10X)
#    define IRMP_PORT_RCC                       CONCAT(RCC_APB2Periph_GPIO, IRMP_PORT_LETTER)
#  elif defined (ARM_STM32F4XX)
#    define IRMP_PORT_RCC                       CONCAT(RCC_AHB1Periph_GPIO, IRMP_PORT_LETTER)
#  endif
#  define IRMP_BIT                              CONCAT(GPIO_Pin_, IRMP_BIT_NUMBER)
#  define IRMP_PIN                              IRMP_PORT   // for use with input(x) below
#  define input(x)                              (GPIO_ReadInputDataBit(x, IRMP_BIT))
#  ifndef USE_STDPERIPH_DRIVER
#    warning The STM32 port of IRMP uses the ST standard peripheral drivers which are not enabled in your build configuration.
#  endif
#endif
#endif

#if IRMP_SUPPORT_DENON_PROTOCOL == 1 && IRMP_SUPPORT_RUWIDO_PROTOCOL == 1
#  warning DENON protocol conflicts wih RUWIDO, please enable only one of both protocols
#  warning RUWIDO protocol disabled
#  undef IRMP_SUPPORT_RUWIDO_PROTOCOL
#  define IRMP_SUPPORT_RUWIDO_PROTOCOL          0
#endif

#if IRMP_SUPPORT_SIEMENS_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, SIEMENS protocol disabled (should be at least 15000)
#  undef IRMP_SUPPORT_SIEMENS_PROTOCOL
#  define IRMP_SUPPORT_SIEMENS_PROTOCOL         0
#endif

#if IRMP_SUPPORT_RUWIDO_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RUWIDO protocol disabled (should be at least 15000)
#  undef IRMP_SUPPORT_RUWIDO_PROTOCOL
#  define IRMP_SUPPORT_RUWIDO_PROTOCOL          0
#endif

#if IRMP_SUPPORT_RECS80_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80 protocol disabled (should be at least 15000)
#  undef IRMP_SUPPORT_RECS80_PROTOCOL
#  define IRMP_SUPPORT_RECS80_PROTOCOL          0
#endif

#if IRMP_SUPPORT_RECS80EXT_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80EXT protocol disabled (should be at least 15000)
#  undef IRMP_SUPPORT_RECS80EXT_PROTOCOL
#  define IRMP_SUPPORT_RECS80EXT_PROTOCOL       0
#endif

#if IRMP_SUPPORT_LEGO_PROTOCOL == 1 && F_INTERRUPTS < 20000
#  warning F_INTERRUPTS too low, LEGO protocol disabled (should be at least 20000)
#  undef IRMP_SUPPORT_LEGO_PROTOCOL
#  define IRMP_SUPPORT_LEGO_PROTOCOL            0
#endif

#if IRMP_SUPPORT_JVC_PROTOCOL == 1 && IRMP_SUPPORT_NEC_PROTOCOL == 0
#  warning JVC protocol needs also NEC protocol, NEC protocol enabled
#  undef IRMP_SUPPORT_NEC_PROTOCOL
#  define IRMP_SUPPORT_NEC_PROTOCOL             1
#endif

#if IRMP_SUPPORT_NEC16_PROTOCOL == 1 && IRMP_SUPPORT_NEC_PROTOCOL == 0
#  warning NEC16 protocol needs also NEC protocol, NEC protocol enabled
#  undef IRMP_SUPPORT_NEC_PROTOCOL
#  define IRMP_SUPPORT_NEC_PROTOCOL             1
#endif

#if IRMP_SUPPORT_NEC42_PROTOCOL == 1 && IRMP_SUPPORT_NEC_PROTOCOL == 0
#  warning NEC42 protocol needs also NEC protocol, NEC protocol enabled
#  undef IRMP_SUPPORT_NEC_PROTOCOL
#  define IRMP_SUPPORT_NEC_PROTOCOL             1
#endif

#if F_INTERRUPTS > 20000
#error F_INTERRUPTS too high (should be not greater than 20000)
#endif

#include "irmpprotocols.h"

#define IRMP_FLAG_REPETITION            0x01

#ifndef IRMP_USE_AS_LIB
extern void                             irmp_init (void);
#endif
extern uint8_t                          irmp_get_data (IRMP_DATA *);
extern uint8_t                          irmp_is_busy (void);
#ifdef IRMP_USE_AS_LIB
extern uint8_t                          irmp_ISR (const uint8_t);
#else
extern uint8_t                          irmp_ISR (void);
#endif
#if IRMP_PROTOCOL_NAMES == 1
extern char *                           irmp_protocol_names[IRMP_N_PROTOCOLS + 1];
#endif

#if IRMP_USE_CALLBACK == 1
extern void                             irmp_set_callback_ptr (void (*cb)(uint8_t));
#endif // IRMP_USE_CALLBACK == 1

#endif /* _IRMP_H_ */
