/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmpconfig.h
 *
 * DO NOT INCLUDE THIS FILE, WILL BE INCLUDED BY IRMP.H!
 *
 * Copyright (c) 2009-2013 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmpconfig.h,v 1.98 2013/01/17 07:33:13 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _IRMPCONFIG_H_
#define _IRMPCONFIG_H_

#ifndef _IRMP_H_
#  error please include only irmp.h, not irmpconfig.h
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change F_INTERRUPTS if you change the number of interrupts per second,
 * Normally, F_INTERRUPTS should be in the range from 10000 to 15000, typical is 15000
 * A value above 15000 costs additional program space, absolute maximum value is 20000.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef F_INTERRUPTS
#  define F_INTERRUPTS                          15000   // interrupts per second, min: 10000, max: 20000, typ: 15000
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change settings from 1 to 0 if you want to disable one or more decoders.
 * This saves program space.
 *
 * 1 enable  decoder
 * 0 disable decoder
 *
 * The standard decoders are enabled per default.
 * Less common protocols are disabled here, you need to enable them manually.
 *
 * If you want to use FDC or RCCAR simultaneous with RC5 protocol, additional program space is required.
 * If you don't need RC5 when using FDC/RCCAR, you should disable RC5.
 * You cannot enable both DENON and RUWIDO, only enable one of them.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

// typical protocols, disable here!             Enable  Remarks                 F_INTERRUPTS            Program Space
#define IRMP_SUPPORT_SIRCS_PROTOCOL             1       // Sony SIRCS           >= 10000                 ~150 bytes
#define IRMP_SUPPORT_NEC_PROTOCOL               1       // NEC + APPLE          >= 10000                 ~300 bytes
#define IRMP_SUPPORT_SAMSUNG_PROTOCOL           1       // Samsung + Samsung32  >= 10000                 ~300 bytes
#define IRMP_SUPPORT_MATSUSHITA_PROTOCOL        1       // Matsushita           >= 10000                  ~50 bytes
#define IRMP_SUPPORT_KASEIKYO_PROTOCOL          1       // Kaseikyo             >= 10000                 ~250 bytes
#define IRMP_SUPPORT_DENON_PROTOCOL             1       // DENON, Sharp         >= 10000                 ~250 bytes

// more protocols, enable here!                 Enable  Remarks                 F_INTERRUPTS            Program Space
#define IRMP_SUPPORT_RC5_PROTOCOL               0       // RC5                  >= 10000                 ~250 bytes
#define IRMP_SUPPORT_RC6_PROTOCOL               0       // RC6 & RC6A           >= 10000                 ~250 bytes
#define IRMP_SUPPORT_JVC_PROTOCOL               0       // JVC                  >= 10000                 ~150 bytes
#define IRMP_SUPPORT_NEC16_PROTOCOL             0       // NEC16                >= 10000                 ~100 bytes
#define IRMP_SUPPORT_NEC42_PROTOCOL             0       // NEC42                >= 10000                 ~300 bytes
#define IRMP_SUPPORT_IR60_PROTOCOL              0       // IR60 (SDA2008)       >= 10000                 ~300 bytes
#define IRMP_SUPPORT_GRUNDIG_PROTOCOL           0       // Grundig              >= 10000                 ~300 bytes
#define IRMP_SUPPORT_SIEMENS_PROTOCOL           0       // Siemens Gigaset      >= 15000                 ~550 bytes
#define IRMP_SUPPORT_NOKIA_PROTOCOL             0       // Nokia                >= 10000                 ~300 bytes

// exotic protocols, enable here!               Enable  Remarks                 F_INTERRUPTS            Program Space
#define IRMP_SUPPORT_BOSE_PROTOCOL              0       // BOSE                 >= 10000                 ~150 bytes
#define IRMP_SUPPORT_KATHREIN_PROTOCOL          0       // Kathrein             >= 10000                 ~200 bytes
#define IRMP_SUPPORT_NUBERT_PROTOCOL            0       // NUBERT               >= 10000                  ~50 bytes
#define IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL      0       // Bang & Olufsen       >= 10000                 ~200 bytes
#define IRMP_SUPPORT_RECS80_PROTOCOL            0       // RECS80 (SAA3004)     >= 15000                  ~50 bytes
#define IRMP_SUPPORT_RECS80EXT_PROTOCOL         0       // RECS80EXT (SAA3008)  >= 15000                  ~50 bytes
#define IRMP_SUPPORT_THOMSON_PROTOCOL           0       // Thomson              >= 10000                 ~250 bytes
#define IRMP_SUPPORT_NIKON_PROTOCOL             0       // NIKON camera         >= 10000                 ~250 bytes
#define IRMP_SUPPORT_NETBOX_PROTOCOL            0       // Netbox keyboard      >= 10000                 ~400 bytes (PROTOTYPE!)
#define IRMP_SUPPORT_FDC_PROTOCOL               0       // FDC3402 keyboard     >= 10000 (better 15000)  ~150 bytes (~400 in combination with RC5)
#define IRMP_SUPPORT_RCCAR_PROTOCOL             0       // RC Car               >= 10000 (better 15000)  ~150 bytes (~500 in combination with RC5)
#define IRMP_SUPPORT_RUWIDO_PROTOCOL            0       // RUWIDO, T-Home       >= 15000                 ~550 bytes
#define IRMP_SUPPORT_A1TVBOX_PROTOCOL           0       // A1 TV BOX            >= 15000 (better 20000)  ~300 bytes
#define IRMP_SUPPORT_LEGO_PROTOCOL              0       // LEGO Power RC        >= 20000                 ~150 bytes


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change hardware pin here for ATMEL AVR
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if defined (ATMEL_AVR)                                                 // use PB6 as IR input on AVR
#  define IRMP_PORT_LETTER                      B
#  define IRMP_BIT_NUMBER                       6

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change hardware pin here for PIC C18 compiler
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#elif defined (PIC_C18)                                                 // use RB4 as IR input on PIC
#  define IRMP_PIN                              PORTBbits.RB4

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change hardware pin here for PIC CCS compiler
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#elif defined (PIC_CCS)
#  define IRMP_PIN                              PIN_B4                  // use PB4 as IR input on PIC

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change hardware pin here for ARM STM32
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#elif defined (ARM_STM32)                                               // use C13 as IR input on STM32
#  define IRMP_PORT_LETTER                      C
#  define IRMP_BIT_NUMBER                       13

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Change hardware pin here for Stellaris ARM Cortex M4
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#elif defined (STELLARIS_ARM_CORTEX_M4)                                 // use B4 as IR input on Stellaris LM4F
#  define IRMP_PORT_LETTER                      B
#  define IRMP_BIT_NUMBER                       4

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Handling of unknown target system: DON'T CHANGE
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#elif !defined (UNIX_OR_WINDOWS)
#  error target system not defined.
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Set IRMP_LOGGING to 1 if want to log data to UART with 9600Bd
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef IRMP_LOGGING
#  define IRMP_LOGGING                          0       // 1: log IR signal (scan), 0: do not. default is 0
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Use external logging routines
 * If you enable external logging, you have also to enable IRMP_LOGGING above
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef IRMP_EXT_LOGGING
#  define IRMP_EXT_LOGGING                      0       // 1: use external logging, 0: do not. default is 0
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Set IRMP_PROTOCOL_NAMES to 1 if want to access protocol names (for logging etc), costs ~300 bytes RAM!
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef IRMP_PROTOCOL_NAMES
#  define IRMP_PROTOCOL_NAMES                   0       // 1: access protocol names, 0: do not. default is 0
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Use Callbacks to indicate input signal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef IRMP_USE_CALLBACK
#  define IRMP_USE_CALLBACK                     0       // 1: use callbacks. 0: do not. default is 0
#endif

#endif /* _WC_IRMPCONFIG_H_ */
