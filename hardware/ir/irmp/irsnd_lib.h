/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irsnd.h
 *
 * Copyright (c) 2010-2011 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irsnd.h,v 1.7 2012/02/16 12:39:36 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _WC_IRSND_H_
#define _WC_IRSND_H_

#if defined(__18CXX)                                        // Microchip C18 declaration of missing typedef
typedef unsigned char               uint8_t;
typedef unsigned int                uint16_t;
#endif  

#define IRSND_NO_REPETITIONS         0      // no repetitions
#define IRSND_MAX_REPETITIONS       14      // max # of repetitions
#define IRSND_ENDLESS_REPETITION    15      // endless repetions
#define IRSND_REPETITION_MASK       0x0F    // lower nibble of flags

/**
 *  Initialize ISND encoder
 *  @details  Configures ISDN output pin
 */
extern void                         irsnd_init (void);

/**
 *  Check if sender is busy
 *  @details  checks if sender is busy
 *  @return    TRUE: sender is busy, FALSE: sender is not busy
 */
extern uint8_t                      irsnd_is_busy (void);

/**
 *  Send IRMP data
 *  @details  sends IRMP data
 *  @param    pointer to IRMP data structure
 *  @return    TRUE: successful, FALSE: failed
 */
extern uint8_t                      irsnd_send_data (IRMP_DATA *, uint8_t);

/**
 *  Stop sending IRMP data
 *  @details  stops sending IRMP data
 */
extern void                         irsnd_stop (void);

/**
 *  ISR routine
 *  @details  ISR routine, called 10000 times per second
 */
extern uint8_t                      irsnd_ISR (void);

#if IRSND_USE_CALLBACK == 1
extern void                         irsnd_set_callback_ptr (void (*cb)(uint8_t));
#endif // IRSND_USE_CALLBACK == 1

#endif /* _WC_IRSND_H_ */
