/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irsnd.h
 *
 * Copyright (c) 2010-2013 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irsnd.h,v 1.16 2013/03/12 12:49:59 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _IRSND_H_
#define _IRSND_H_

#include "irmpsystem.h"
#ifndef IRSND_USE_AS_LIB
#  include "irsndconfig.h"
#endif

#if defined (ARM_STM32)                         // STM32
#  define _CONCAT(a,b)                          a##b
#  define CONCAT(a,b)                           _CONCAT(a,b)
#  define IRSND_PORT                            CONCAT(GPIO, IRSND_PORT_LETTER)
#  if defined (ARM_STM32L1XX)
#    define IRSND_PORT_RCC                      CONCAT(RCC_AHBPeriph_GPIO, IRSND_PORT_LETTER)
#    define IRSND_GPIO_AF                       CONCAT(GPIO_AF_TIM, IRSND_TIMER_NUMBER)
#  elif defined (ARM_STM32F10X)
#    define IRSND_PORT_RCC                      CONCAT(RCC_APB2Periph_GPIO, IRSND_PORT_LETTER)
#  elif defined (ARM_STM32F4XX)
#    define IRSND_PORT_RCC                      CONCAT(RCC_AHB1Periph_GPIO, IRSND_PORT_LETTER)
#    define IRSND_GPIO_AF                       CONCAT(GPIO_AF_TIM, IRSND_TIMER_NUMBER)
#  endif
#  define IRSND_BIT                             CONCAT(GPIO_Pin_, IRSND_BIT_NUMBER)
#  define IRSND_TIMER                           CONCAT(TIM, IRSND_TIMER_NUMBER)
#  define IRSND_TIMER_CHANNEL                   CONCAT(TIM_Channel_, IRSND_TIMER_CHANNEL_NUMBER)
#  if ((IRSND_TIMER_NUMBER >= 2) && (IRSND_TIMER_NUMBER <= 5)) || ((IRSND_TIMER_NUMBER >= 12) && (IRSND_TIMER_NUMBER <= 14))
#    define IRSND_TIMER_RCC                     CONCAT(RCC_APB1Periph_TIM, IRSND_TIMER_NUMBER)
#  elif (IRSND_TIMER_NUMBER == 1) || ((IRSND_TIMER_NUMBER >= 8) && (IRSND_TIMER_NUMBER <= 11))
#    define IRSND_TIMER_RCC                     CONCAT(RCC_APB2Periph_TIM, IRSND_TIMER_NUMBER)
#  else
#    error IRSND_TIMER_NUMBER not valid.
#  endif
#  ifndef USE_STDPERIPH_DRIVER
#    warning The STM32 port of IRSND uses the ST standard peripheral drivers which are not enabled in your build configuration.
#  endif

#elif defined(PIC_C18)
// Do not change lines below until you have a different HW. Example is for 18F2550/18F4550
// setup macro for PWM used PWM module
#  if IRSND_OCx == IRSND_PIC_CCP2        
#    define IRSND_PIN                           TRISCbits.TRISC1        // RC1 = PWM2
#    define SetDCPWM(x)                         SetDCPWM2(x)                    
#    define ClosePWM                            ClosePWM2
#    define OpenPWM(x)                          OpenPWM2(x) 
#  endif
#  if IRSND_OCx == IRSND_PIC_CCP1        
#    define IRSND_PIN                           TRISCbits.TRISC2        // RC2 = PWM1
#    define SetDCPWM(x)                         SetDCPWM1(x)
#    define ClosePWM                            ClosePWM1
#    define OpenPWM(x)                          OpenPWM1(x)
#  endif
//Setup macro for OpenTimer with defined Pre_Scaler
#  if Pre_Scaler == 1
#    define OpenTimer                               OpenTimer2(TIMER_INT_OFF & T2_PS_1_1); 
#  elif Pre_Scaler == 4
#    define OpenTimer                               OpenTimer2(TIMER_INT_OFF & T2_PS_1_4); 
#  elif Pre_Scaler == 16
#    define OpenTimer                               OpenTimer2(TIMER_INT_OFF & T2_PS_1_16); 
#  else
#    error Incorrect value for Pre_Scaler
#  endif
#endif // ARM_STM32

#if IRSND_SUPPORT_SIEMENS_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, SIEMENS protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_SIEMENS_PROTOCOL
#  define IRSND_SUPPORT_SIEMENS_PROTOCOL        0
#endif

#if IRSND_SUPPORT_A1TVBOX_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, A1TVBOX protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_A1TVBOX_PROTOCOL
#  define IRSND_SUPPORT_A1TVBOX_PROTOCOL        0
#endif

#if IRSND_SUPPORT_RECS80_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80 protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_RECS80_PROTOCOL
#  define IRSND_SUPPORT_RECS80_PROTOCOL         0
#endif

#if IRSND_SUPPORT_RECS80EXT_PROTOCOL == 1 && F_INTERRUPTS < 15000
#  warning F_INTERRUPTS too low, RECS80EXT protocol disabled (should be at least 15000)
#  undef IRSND_SUPPORT_RECS80EXT_PROTOCOL
#  define IRSND_SUPPORT_RECS80EXT_PROTOCOL      0
#endif

#if IRSND_SUPPORT_LEGO_PROTOCOL == 1 && F_INTERRUPTS < 20000
#  warning F_INTERRUPTS too low, LEGO protocol disabled (should be at least 20000)
#  undef IRSND_SUPPORT_LEGO_PROTOCOL
#  define IRSND_SUPPORT_LEGO_PROTOCOL           0
#endif

#include "irmpprotocols.h"

#define IRSND_NO_REPETITIONS                     0      // no repetitions
#define IRSND_MAX_REPETITIONS                   14      // max # of repetitions
#define IRSND_ENDLESS_REPETITION                15      // endless repetions
#define IRSND_REPETITION_MASK                   0x0F    // lower nibble of flags

extern void                                     irsnd_init (void);
extern uint8_t                                  irsnd_is_busy (void);
extern uint8_t                                  irsnd_send_data (IRMP_DATA *, uint8_t);
extern void                                     irsnd_stop (void);
extern uint8_t                                  irsnd_ISR (void);

#if IRSND_USE_CALLBACK == 1
extern void                                     irsnd_set_callback_ptr (void (*cb)(uint8_t));
#endif // IRSND_USE_CALLBACK == 1

#endif /* _IRSND_H_ */
