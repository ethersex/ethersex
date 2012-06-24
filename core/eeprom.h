/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright(c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _EEPROM_H
#define _EEPROM_H

#include <stdint.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include "config.h"
#include "core/global.h"
#include "protocols/uip/uip.h"

#define IPADDR_LEN sizeof(uip_ipaddr_t)

#ifdef STELLA_SUPPORT
#include "services/stella/stella.h"
#endif

#ifdef SMS77_EEPROM_SUPPORT
#include "protocols/sms77/sms77.h"
#endif

#ifdef JABBER_EEPROM_SUPPORT
#include "services/jabber/jabber.h"
#endif

#ifdef MOTD_SUPPORT
#include "services/motd/motd.h"
#endif

#ifdef DMX_FXSLOT_SUPPORT
#include "services/dmx-fxslot/dmx-fxslot.h"
#endif

#ifdef ADC_VOLTAGE_SUPPORT
#include "hardware/adc/adc.h"
#endif

#ifdef ONEWIRE_NAMING_SUPPORT
#include "hardware/onewire/onewire.h"
#endif

#ifdef CRON_EEPROM_SUPPORT
#include "services/cron/cron.h"
#endif

#ifdef TANKLEVEL_SUPPORT
#include "services/tanklevel/tanklevel.h"
#endif

struct eeprom_config_t
{
#ifdef ETHERNET_SUPPORT
  uint8_t mac[6];
#endif

#if (defined(IPV4_SUPPORT) && !defined(BOOTP_SUPPORT)) || defined(IPV6_STATIC_SUPPORT)
  uint8_t ip[IPADDR_LEN];
#endif

#ifdef IPV4_SUPPORT
  uint8_t netmask[IPADDR_LEN];
#endif

  uint8_t gateway[IPADDR_LEN];

#ifdef DNS_SUPPORT
  uint8_t dns_server[IPADDR_LEN];
#endif

#ifdef NTP_SUPPORT
  uint8_t ntp_server[IPADDR_LEN];
#endif

#ifdef PAM_SINGLE_USER_EEPROM_SUPPORT
  char pam_username[16];
  char pam_password[16];
#endif

#ifdef ADC_VOLTAGE_SUPPORT
  uint16_t adc_vref;
#endif

#ifdef KTY_SUPPORT
  int8_t kty_calibration;
#endif

#ifdef STELLA_SUPPORT
  uint8_t stella_channel_values[STELLA_CHANNELS];
#endif

#ifdef SMS77_EEPROM_SUPPORT
  char sms77_username[SMS77_VALUESIZE];
  char sms77_password[SMS77_VALUESIZE];
  char sms77_receiver[SMS77_VALUESIZE];
  char sms77_type[SMS77_VALUESIZE];
#endif

#ifdef JABBER_EEPROM_SUPPORT
  char jabber_username[JABBER_VALUESIZE];
  char jabber_password[JABBER_VALUESIZE];
  char jabber_resource[JABBER_VALUESIZE];
  char jabber_hostname[JABBER_VALUESIZE];
#endif

#ifdef MOTD_SUPPORT
  char motd_text[MOTD_VALUESIZE];
#endif

#ifdef DMX_FXSLOT_SUPPORT
  struct fxslot_struct_stripped dmx_fxslots[DMX_FXSLOT_AMOUNT];
#endif

#ifdef ONEWIRE_NAMING_SUPPORT
  ow_name_t ow_names[OW_SENSORS_COUNT];
#endif

#ifdef CRON_EEPROM_SUPPORT
  uint8_t crontab[CRON_EEPROM_SIZE];
#endif

#ifdef TANKLEVEL_SUPPORT
  tanklevel_params_t tanklevel_params;
#endif
  uint8_t crc;
};


#define EEPROM_CONFIG_BASE  (uint8_t *)0x0000


uint8_t crc_checksum (void *data, uint8_t length);
void eeprom_write_block_hack (void *dst, const void *src, size_t n);

/* Reset the EEPROM to sane defaults. */
void eeprom_reset (void);

/* Initialize EEPROM cruft. */
void eeprom_init (void);

/* Calculate crc value, from config saved in eeprom */
uint8_t eeprom_get_chksum (void);

#define eeprom_save(dst, data, len) \
  eeprom_write_block_hack(EEPROM_CONFIG_BASE + offsetof(struct eeprom_config_t, dst), data, len)

#define eeprom_save_offset(dst, off, data, len) \
  eeprom_write_block_hack(EEPROM_CONFIG_BASE + offsetof(struct eeprom_config_t, dst) + off, data, len)

#define eeprom_save_P(dst,data_pgm,len) \
    do { char data[len]; memcpy_P(data, data_pgm, len); eeprom_save(dst, data, len);} while(0)

#define eeprom_save_char(dst,data) \
    do { uint8_t _t = data; eeprom_save(dst, &_t, 1); } while(0)

#define eeprom_save_int(dst, data) \
    do { uint16_t _t = data; eeprom_save(dst, &_t, 2); } while(0)

/* Reads len byte from eeprom at dst into mem */
#define eeprom_restore(dst, mem, len) \
  eeprom_read_block(mem, EEPROM_CONFIG_BASE + offsetof(struct eeprom_config_t, dst), len)

#define eeprom_restore_offset(dst, off, mem, len) \
  eeprom_read_block(mem, EEPROM_CONFIG_BASE + offsetof(struct eeprom_config_t, dst) + off, len)

#define eeprom_restore_ip(dst,mem) \
    eeprom_restore(dst, mem, IPADDR_LEN)

#define eeprom_restore_char(dst,mem) \
    eeprom_restore(dst, mem, 1)

#define eeprom_restore_int(dst, mem) \
    eeprom_restore(dst, mem, 2)

/* Update the eeprom crc */
#define eeprom_update_chksum() eeprom_save_char(crc, eeprom_get_chksum())



#endif /* _EEPROM_H */
