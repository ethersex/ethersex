/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 * Copyright (c) 2010 by Jens Wilmer <ethersex@jenswilmer.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

// Various strings used for building paths
// First part of the path up to the pin identifier
static const char PROGMEM watchasync_path[] =
    CONF_WATCHASYNC_METHOD " " CONF_WATCHASYNC_PATH ;

// next is the optional part between pin identifier and timestamp
#ifdef CONF_WATCHASYNC_TIMESTAMP
static const char PROGMEM watchasync_timestamp_path[] =
	CONF_WATCHASYNC_TIMESTAMP_PATH ;
#endif

#ifdef CONF_WATCHASYNC_SUMMARIZE
static const char PROGMEM watchasync_summarize_path[] =
        CONF_WATCHASYNC_SUMMARIZE_PATH ;
#endif // def CONF_WATCHASYNC_SUMMARIZE

// and the http footer including the http protocol version and the server name
static const char PROGMEM watchasync_request_end[] =
    CONF_WATCHASYNC_END_PATH " HTTP/1.1\r\n"
    "Host: " CONF_WATCHASYNC_SERVER "\r\n"
    "Content-Length: 0\r\n\r\n";

#ifndef CONF_WATCHASYNC_PORT
#define CONF_WATCHASYNC_PORT 80
#endif

#ifdef CONF_WATCHASYNC_PA0
static const char PROGMEM watchasync_ID_PA0[] = CONF_WATCHASYNC_PA0_ID;
#endif
#ifdef CONF_WATCHASYNC_PA1
static const char PROGMEM watchasync_ID_PA1[] = CONF_WATCHASYNC_PA1_ID;
#endif
#ifdef CONF_WATCHASYNC_PA2
static const char PROGMEM watchasync_ID_PA2[] = CONF_WATCHASYNC_PA2_ID;
#endif
#ifdef CONF_WATCHASYNC_PA3
static const char PROGMEM watchasync_ID_PA3[] = CONF_WATCHASYNC_PA3_ID;
#endif
#ifdef CONF_WATCHASYNC_PA4
static const char PROGMEM watchasync_ID_PA4[] = CONF_WATCHASYNC_PA4_ID;
#endif
#ifdef CONF_WATCHASYNC_PA5
static const char PROGMEM watchasync_ID_PA5[] = CONF_WATCHASYNC_PA5_ID;
#endif
#ifdef CONF_WATCHASYNC_PA6
static const char PROGMEM watchasync_ID_PA6[] = CONF_WATCHASYNC_PA6_ID;
#endif
#ifdef CONF_WATCHASYNC_PA7
static const char PROGMEM watchasync_ID_PA7[] = CONF_WATCHASYNC_PA7_ID;
#endif
#ifdef CONF_WATCHASYNC_PB0
static const char PROGMEM watchasync_ID_PB0[] = CONF_WATCHASYNC_PB0_ID;
#endif
#ifdef CONF_WATCHASYNC_PB1
static const char PROGMEM watchasync_ID_PB1[] = CONF_WATCHASYNC_PB1_ID;
#endif
#ifdef CONF_WATCHASYNC_PB2
static const char PROGMEM watchasync_ID_PB2[] = CONF_WATCHASYNC_PB2_ID;
#endif
#ifdef CONF_WATCHASYNC_PB3
static const char PROGMEM watchasync_ID_PB3[] = CONF_WATCHASYNC_PB3_ID;
#endif
#ifdef CONF_WATCHASYNC_PB4
static const char PROGMEM watchasync_ID_PB4[] = CONF_WATCHASYNC_PB4_ID;
#endif
#ifdef CONF_WATCHASYNC_PB5
static const char PROGMEM watchasync_ID_PB5[] = CONF_WATCHASYNC_PB5_ID;
#endif
#ifdef CONF_WATCHASYNC_PB6
static const char PROGMEM watchasync_ID_PB6[] = CONF_WATCHASYNC_PB6_ID;
#endif
#ifdef CONF_WATCHASYNC_PB7
static const char PROGMEM watchasync_ID_PB7[] = CONF_WATCHASYNC_PB7_ID;
#endif
#ifdef CONF_WATCHASYNC_PC0
static const char PROGMEM watchasync_ID_PC0[] = CONF_WATCHASYNC_PC0_ID;
#endif
#ifdef CONF_WATCHASYNC_PC1
static const char PROGMEM watchasync_ID_PC1[] = CONF_WATCHASYNC_PC1_ID;
#endif
#ifdef CONF_WATCHASYNC_PC2
static const char PROGMEM watchasync_ID_PC2[] = CONF_WATCHASYNC_PC2_ID;
#endif
#ifdef CONF_WATCHASYNC_PC3
static const char PROGMEM watchasync_ID_PC3[] = CONF_WATCHASYNC_PC3_ID;
#endif
#ifdef CONF_WATCHASYNC_PC4
static const char PROGMEM watchasync_ID_PC4[] = CONF_WATCHASYNC_PC4_ID;
#endif
#ifdef CONF_WATCHASYNC_PC5
static const char PROGMEM watchasync_ID_PC5[] = CONF_WATCHASYNC_PC5_ID;
#endif
#ifdef CONF_WATCHASYNC_PC6
static const char PROGMEM watchasync_ID_PC6[] = CONF_WATCHASYNC_PC6_ID;
#endif
#ifdef CONF_WATCHASYNC_PC7
static const char PROGMEM watchasync_ID_PC7[] = CONF_WATCHASYNC_PC7_ID;
#endif
#ifdef CONF_WATCHASYNC_PD0
static const char PROGMEM watchasync_ID_PD0[] = CONF_WATCHASYNC_PD0_ID;
#endif
#ifdef CONF_WATCHASYNC_PD1
static const char PROGMEM watchasync_ID_PD1[] = CONF_WATCHASYNC_PD1_ID;
#endif
#ifdef CONF_WATCHASYNC_PD2
static const char PROGMEM watchasync_ID_PD2[] = CONF_WATCHASYNC_PD2_ID;
#endif
#ifdef CONF_WATCHASYNC_PD3
static const char PROGMEM watchasync_ID_PD3[] = CONF_WATCHASYNC_PD3_ID;
#endif
#ifdef CONF_WATCHASYNC_PD4
static const char PROGMEM watchasync_ID_PD4[] = CONF_WATCHASYNC_PD4_ID;
#endif
#ifdef CONF_WATCHASYNC_PD5
static const char PROGMEM watchasync_ID_PD5[] = CONF_WATCHASYNC_PD5_ID;
#endif
#ifdef CONF_WATCHASYNC_PD6
static const char PROGMEM watchasync_ID_PD6[] = CONF_WATCHASYNC_PD6_ID;
#endif
#ifdef CONF_WATCHASYNC_PD7
static const char PROGMEM watchasync_ID_PD7[] = CONF_WATCHASYNC_PD7_ID;
#endif

const PGM_P const watchasync_ID[] PROGMEM =
{
#ifdef CONF_WATCHASYNC_PA0
  watchasync_ID_PA0
#if WATCHASYNC_PA0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA1
  watchasync_ID_PA1
#if WATCHASYNC_PA1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA2
  watchasync_ID_PA2
#if WATCHASYNC_PA2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA3
  watchasync_ID_PA3
#if WATCHASYNC_PA3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA4
  watchasync_ID_PA4
#if WATCHASYNC_PA4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA5
  watchasync_ID_PA5
#if WATCHASYNC_PA5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA6
  watchasync_ID_PA6
#if WATCHASYNC_PA6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA7
  watchasync_ID_PA7
#if WATCHASYNC_PA7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB0
  watchasync_ID_PB0
#if WATCHASYNC_PB0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB1
  watchasync_ID_PB1
#if WATCHASYNC_PB1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB2
  watchasync_ID_PB2
#if WATCHASYNC_PB2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB3
  watchasync_ID_PB3
#if WATCHASYNC_PB3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB4
  watchasync_ID_PB4
#if WATCHASYNC_PB4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB5
  watchasync_ID_PB5
#if WATCHASYNC_PB5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB6
  watchasync_ID_PB6
#if WATCHASYNC_PB6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB7
  watchasync_ID_PB7
#if WATCHASYNC_PB7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC0
  watchasync_ID_PC0
#if WATCHASYNC_PC0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC1
  watchasync_ID_PC1
#if WATCHASYNC_PC1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC2
  watchasync_ID_PC2
#if WATCHASYNC_PC2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC3
  watchasync_ID_PC3
#if WATCHASYNC_PC3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC4
  watchasync_ID_PC4
#if WATCHASYNC_PC4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC5
  watchasync_ID_PC5
#if WATCHASYNC_PC5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC6
  watchasync_ID_PC6
#if WATCHASYNC_PC6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC7
  watchasync_ID_PC7
#if WATCHASYNC_PC7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD0
  watchasync_ID_PD0
#if WATCHASYNC_PD0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD1
  watchasync_ID_PD1
#if WATCHASYNC_PD1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD2
  watchasync_ID_PD2
#if WATCHASYNC_PD2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD3
  watchasync_ID_PD3
#if WATCHASYNC_PD3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD4
  watchasync_ID_PD4
#if WATCHASYNC_PD4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD5
  watchasync_ID_PD5
#if WATCHASYNC_PD5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD6
  watchasync_ID_PD6
#if WATCHASYNC_PD6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD7
  watchasync_ID_PD7
#endif
};


