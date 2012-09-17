/*
* Periodically blink heartbeat / activity LED
*
* Copyright (c) 2010 by Gerd v. Egidy <gerd@egidy.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
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

#ifndef _HEARTBEAT_H
#define _HEARTBEAT_H

void heartbeat_init(void);
void heartbeat_periodic(void);

#ifdef STATUSLED_RFM12_TX_ACT_SUPPORT
    #define ACTIVITY_LED_RFM12_TX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_RFM12_TX do { } while(0)
#endif

#ifdef STATUSLED_RFM12_RX_ACT_SUPPORT
    #define ACTIVITY_LED_RFM12_RX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_RFM12_RX do { } while(0)
#endif

#ifdef STATUSLED_ZBUS_TX_ACT_SUPPORT
    #define ACTIVITY_LED_ZBUS_TX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_ZBUS_TX do { } while(0)
#endif

#ifdef STATUSLED_ZBUS_RX_ACT_SUPPORT
    #define ACTIVITY_LED_ZBUS_RX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_ZBUS_RX do { } while(0)
#endif

#ifdef STATUSLED_EMS_TX_ACT_SUPPORT
    #define ACTIVITY_LED_EMS_TX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_EMS_TX do { } while(0)
#endif

#ifdef STATUSLED_EMS_RX_ACT_SUPPORT
    #define ACTIVITY_LED_EMS_RX PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_EMS_RX do { } while(0)
#endif

#ifdef STATUSLED_ECMD_ACT_SUPPORT
    #define ACTIVITY_LED_ECMD PIN_TOGGLE(STATUSLED_HB_ACT)
#else
    #define ACTIVITY_LED_ECMD do { } while(0)
#endif


#endif
