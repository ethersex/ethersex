/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright(c) 2008 Stefan Siegl <stesie@brokenpipe.de>
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

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/eeprom.h"


#ifdef CRC_SUPPORT
uint8_t
crc_checksum (void *data, uint8_t length)
{

  uint8_t crc = 0;
  uint8_t *p = (uint8_t *) data;

  for (uint8_t i = 0; i < length; i++)
    {
      crc = _crc_ibutton_update (crc, *p);
      p++;
    }

  return crc;

}
#endif /* CRC_SUPPORT */


#ifdef EEPROM_SUPPORT

void
eeprom_write_block_hack (void *dst, const void *src, size_t n)
{
  while (n--)
    {
      eeprom_write_byte (&((uint8_t *) dst)[n], ((uint8_t *) src)[n]);
    }
}


uint8_t
eeprom_get_chksum (void)
{
  uint8_t eeprom_crc = 0;
  uint8_t *p = (uint8_t *) EEPROM_CONFIG_BASE;

  for (uint16_t i = 0; i < (sizeof (struct eeprom_config_t) - 1); i++)
    {
      eeprom_crc = _crc_ibutton_update (eeprom_crc, eeprom_read_byte (p));
      p++;
    }

  return eeprom_crc;
}



void
eeprom_init (void)
{
  uip_ipaddr_t ip;
  (void) ip;			/* Keep GCC quiet. */

#ifdef ETHERNET_SUPPORT
  eeprom_save_P (mac, PSTR (CONF_ETHERSEX_MAC), 6);
#endif

#if (defined(IPV4_SUPPORT) && !defined(BOOTP_SUPPORT) && !defined(DHCP_SUPPORT)) || defined(IPV6_STATIC_SUPPORT)
  set_CONF_ETHERSEX_IP (&ip);
  eeprom_save (ip, &ip, IPADDR_LEN);
#ifdef ETHERNET_SUPPORT
  set_CONF_ETHERSEX_GATEWAY (&ip);
  eeprom_save (gateway, &ip, IPADDR_LEN);
#endif

#ifdef IPV4_SUPPORT
  set_CONF_ETHERSEX_IP4_NETMASK (&ip);
  eeprom_save (netmask, &ip, IPADDR_LEN);
#endif
#endif

#ifdef DNS_SUPPORT
  set_CONF_DNS_SERVER (&ip);
  eeprom_save (dns_server, &ip, IPADDR_LEN);
#endif

#ifdef PAM_SINGLE_USER_EEPROM_SUPPORT
  /* Copy the httpd's password. */
  eeprom_save_P (pam_username, PSTR (PAM_SINGLE_USERNAME), 16);
  eeprom_save_P (pam_password, PSTR (PAM_SINGLE_PASSWORD), 16);
#endif

#ifdef ADC_VOLTAGE_SUPPORT
  eeprom_save_int (adc_vref, ADC_REF_VOLTAGE);
#endif

#ifdef KTY_SUPPORT
  eeprom_save_char (kty_calibration, 0);
#endif

#ifdef STELLA_EEPROM
  uint8_t stella_temp[10] = { 0 };
  eeprom_save (stella_channel_values, stella_temp, 10);
#endif

#ifdef DMX_FXSLOT_SUPPORT
  struct fxslot_struct_stripped fxslots_temp[DMX_FXSLOT_AMOUNT] = { {0,0,0,0,0,0,0} };
  eeprom_save (dmx_fxslots, fxslots_temp, DMX_FXSLOT_AMOUNT*sizeof(struct fxslot_struct_stripped));
#endif

#ifdef ONEWIRE_NAMING_SUPPORT
  ow_name_t temp_name;
  memset(&temp_name, 0, sizeof(ow_name_t));
  for (int8_t i = 0; i < OW_SENSORS_COUNT; i++)
  {
    eeprom_save(ow_names[i], &temp_name, sizeof(ow_name_t));
  }
#endif

#ifdef SMS77_EEPROM_SUPPORT
  eeprom_save_P (sms77_username, PSTR (CONF_SMS77_USER), SMS77_VALUESIZE);
  eeprom_save_P (sms77_password, PSTR (CONF_SMS77_PASS), SMS77_VALUESIZE);
  eeprom_save_P (sms77_receiver, PSTR (CONF_SMS77_TO), SMS77_VALUESIZE);
  eeprom_save_P (sms77_type, PSTR (CONF_SMS77_TYPE), SMS77_VALUESIZE);
#endif

#ifdef JABBER_EEPROM_SUPPORT
  eeprom_save_P (jabber_username, PSTR (CONF_JABBER_USERNAME),
		 JABBER_VALUESIZE);
  eeprom_save_P (jabber_password, PSTR (CONF_JABBER_PASSWORD),
		 JABBER_VALUESIZE);
  eeprom_save_P (jabber_resource, PSTR (CONF_JABBER_RESOURCE),
		 JABBER_VALUESIZE);
  eeprom_save_P (jabber_hostname, PSTR (CONF_JABBER_HOSTNAME),
		 JABBER_VALUESIZE);
#endif

#ifdef MOTD_SUPPORT
  eeprom_save_P (motd_text, PSTR (CONF_MOTD_DEFAULT), MOTD_VALUESIZE);
#endif

#ifdef CRON_EEPROM_SUPPORT
  uint8_t count = 0;
  eeprom_save_offset(crontab, 0, &count, sizeof(count));
#endif

#ifdef TANKLEVEL_SUPPORT
  tanklevel_params_t tanklevel_temp = {
    .sensor_offset = TANKLEVEL_SENSOR_OFFSET,
    .med_density = TANKLEVEL_MED_DENSITY,
    .ltr_per_m = TANKLEVEL_LTR_PER_M,
    .ltr_full = TANKLEVEL_LTR_FULL,
    .raise_time = TANKLEVEL_RAISE_TIME,
    .hold_time = TANKLEVEL_HOLD_TIME
  };
  eeprom_save (tanklevel_params, &tanklevel_temp, sizeof(tanklevel_params_t));
#endif
  eeprom_update_chksum ();
}


#endif /* EEPROM_SUPPORT */
