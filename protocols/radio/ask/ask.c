/*
 * Copyright (c) Gregor B.
 * Copyright (c) Guido Pannenbecker
 * Copyright (c) Stefan Riepenhausen
 * Copyright (c) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2012-15 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "config.h"

#include "ask.h"


#define ASK_HARDWARE_RFM12     1
#define ASK_HARDWARE_GENERIC   2

#if ASK_HARDWARE == ASK_HARDWARE_RFM12
#include "hardware/radio/rfm12/rfm12_ask.h"
#elif ASK_HARDWARE == ASK_HARDWARE_GENERIC
#include "hardware/radio/generic/generic.h"
#else
#error wrong value for ASK_HARDWARE
#endif


#define INTERTECHNO_PERIOD     264      // produces pulse of 360 us
#define INTERTECHNO_SL_PERIOD  216


#ifdef PROTO_TEVION_SUPPORT
void
ask_tevion_send(uint8_t * housecode, uint8_t * command, uint8_t delay,
                uint8_t cnt)
{
  uint8_t code[41];
  uint8_t *p = code;

  for (uint8_t i = 0; i < 3; i++)
  {
    uint8_t byte = housecode[i];
    for (uint8_t mask = 0x80; mask; mask >>= 1)
    {
      *p++ = byte & mask ? 12 : 6;
    }
  }
  *p++ = housecode[2] & 1 ? 6 : 12;
  for (uint8_t i = 0; i < 2; i++)
  {
    uint8_t byte = command[i];
    for (uint8_t mask = 0x80; mask; mask >>= 1)
    {
      *p++ = byte & mask ? 12 : 6;
    }
  }

  ASK_TX_ENABLE;

  for (uint8_t ii = cnt; ii > 0; ii--)
  {
    wdt_kick();
    uint8_t ask_trigger_level = 0;
    for (uint8_t i = 0; i < 41; i++)
    {
      ASK_TX_TRIGGER(ask_trigger_level ^= 1, code[i] * delay);
    }
    ASK_TX_TRIGGER(0, 24 * delay);
  }

  ASK_TX_DISABLE;
}
#endif /* PROTO_TEVION_SUPPORT */

#ifdef PROTO_INTERTECHNO_SUPPORT
static void
ask_intertechno_send_bit(const uint8_t bit)
{
  ASK_TX_TRIGGER(1, INTERTECHNO_PERIOD);
  ASK_TX_TRIGGER(0, 3 * INTERTECHNO_PERIOD);
  if (bit)
  {
    ASK_TX_TRIGGER(1, 3 * INTERTECHNO_PERIOD);
    ASK_TX_TRIGGER(0, INTERTECHNO_PERIOD);
  }
  else
  {
    ASK_TX_TRIGGER(1, INTERTECHNO_PERIOD);
    ASK_TX_TRIGGER(0, 3 * INTERTECHNO_PERIOD);
  }
}

static void
ask_intertechno_send_sync(void)
{
  ASK_TX_TRIGGER(1, INTERTECHNO_PERIOD);
  ASK_TX_TRIGGER(0, 31 * INTERTECHNO_PERIOD);
}

void
ask_intertechno_send(uint8_t family, uint8_t group,
                     uint8_t device, uint8_t command)
{
  union
  {
    struct
    {
      uint16_t family:4;
      uint16_t device:2;
      uint16_t group:2;
      uint16_t command:4;
    } bits;
    uint16_t raw;
  } code;

  family -= 1;
  code.bits.family = family;
  device -= 1;
  code.bits.device = device;
  group -= 1;
  code.bits.group = group;
  code.bits.command = command ? 0x0E : 0x06;

  ASK_TX_ENABLE;

  for (uint8_t j = 6; j > 0; j--)
  {
    wdt_kick();
    uint16_t c = code.raw;
    for (uint8_t i = 12; i; i--)
    {
      ask_intertechno_send_bit(c & 1);
      c >>= 1;
    }
    ask_intertechno_send_sync();
  }

  ASK_TX_DISABLE;
}
#endif /* PROTO_INTERTECHNO_SUPPORT */

#ifdef PROTO_INTERTECHNO_SL_SUPPORT

static void
ask_intertechno_sl_send_bit(const uint8_t bit)
{
  ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
  if (bit)
  {
    ASK_TX_TRIGGER(0, 5 * INTERTECHNO_SL_PERIOD);
    ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
    ASK_TX_TRIGGER(0, INTERTECHNO_SL_PERIOD);
  }
  else
  {
    ASK_TX_TRIGGER(0, INTERTECHNO_SL_PERIOD);
    ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
    ASK_TX_TRIGGER(0, 5 * INTERTECHNO_SL_PERIOD);
  }
}

static void
ask_intertechno_sl_send_dim(void)
{
  ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
  ASK_TX_TRIGGER(0, INTERTECHNO_SL_PERIOD);
  ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
  ASK_TX_TRIGGER(0, INTERTECHNO_SL_PERIOD);
}

static void
ask_intertechno_sl_send_sync(void)
{
  ASK_TX_TRIGGER(1, INTERTECHNO_SL_PERIOD);
  ASK_TX_TRIGGER(0, 10 * INTERTECHNO_SL_PERIOD);
}

static void
ask_intertechno_sl_send_pause(void)
{
  ASK_TX_TRIGGER(0, 40 * INTERTECHNO_SL_PERIOD);
}

void
ask_intertechno_sl_send(uint32_t house,
                        uint8_t on, uint8_t button, int8_t dim)
{
  ASK_TX_ENABLE;

  for (uint8_t j = 8; j > 0; j--)
  {
    wdt_kick();
    ask_intertechno_sl_send_sync();

    for (int8_t i = 25; i >= 0; i--)
    {
      ask_intertechno_sl_send_bit(house & 1 << i);
    }
    ask_intertechno_sl_send_bit(0);     //Group

    if (on == 1 && dim != -1)
    {
      ask_intertechno_sl_send_dim();
    }
    else
    {
      ask_intertechno_sl_send_bit(on & 1);
    }
    for (int8_t i = 3; i >= 0; i--)
    {
      ask_intertechno_sl_send_bit(button & 1 << i);
    }
    if (dim != -1)
    {
      for (int8_t i = 3; i >= 0; i--)
      {
        ask_intertechno_sl_send_bit(dim & 1 << i);
      }
    }
    ask_intertechno_sl_send_bit(0);
    ask_intertechno_sl_send_pause();
  }

  ASK_TX_DISABLE;
}

#endif /* PROTO_INTERTECHNO_SL_SUPPORT */

#if defined PROTO_2272_SUPPORT || defined PROTO_1527_SUPPORT

#ifdef PROTO_2272_SUPPORT
static const uint8_t ask_2272_pulse_duty_factor[4] PROGMEM = { 13, 5, 7, 11 };
#endif
#ifdef PROTO_1527_SUPPORT
static const uint8_t ask_1527_pulse_duty_factor[4] PROGMEM = { 9, 3, 3, 9 };
#endif

static void
ask_2272_1527_send(uint8_t * command, uint8_t delay, uint8_t cnt,
                   const uint8_t * duty_factor, uint8_t sync)
{
  uint8_t code[49];
  uint8_t *p = code;

  for (uint8_t i = 0; i < 3; i++)
  {
    uint8_t byte = command[i];
    for (uint8_t mask = 0x80; mask; mask >>= 1)
    {
      if (byte & mask)
      {
        *p++ = pgm_read_byte(duty_factor);
        *p++ = pgm_read_byte(duty_factor + 1);
      }
      else
      {
        *p++ = pgm_read_byte(duty_factor + 2);
        *p++ = pgm_read_byte(duty_factor + 3);
      }
    }
  }
  *p = 7;                       // sync

  ASK_TX_ENABLE;

  for (uint8_t ii = cnt; ii > 0; ii--)
  {
    wdt_kick();
    uint8_t ask_trigger_level = 0;
    for (uint8_t i = 0; i < 49; i++)
    {
      ASK_TX_TRIGGER(ask_trigger_level ^= 1, code[i] * delay);
    }
    ASK_TX_TRIGGER(0, sync * delay);
  }

  ASK_TX_DISABLE;
}

#ifdef PROTO_2272_SUPPORT
void
ask_2272_send(uint8_t * command, uint8_t delay, uint8_t cnt, uint8_t sync)
{
  ask_2272_1527_send(command, delay, cnt, ask_2272_pulse_duty_factor, sync);
}
#endif

#ifdef PROTO_1527_SUPPORT
void
ask_1527_send(uint8_t * command, uint8_t delay, uint8_t cnt)
{
  ask_2272_1527_send(command, delay, cnt, ask_1527_pulse_duty_factor, 24);
}
#endif
#endif /* PROTO_2272_SUPPORT || PROTO_1527_SUPPORT */

#ifdef PROTO_OASEFMMASTER_SUPPORT
void
ask_oase_send(uint8_t * command, uint8_t delay, uint8_t cnt)
{
  uint8_t code[51];
  uint8_t *p = code;

  /* Eine 0 im voraus, die immer bleibt, so dass Befehl trotzdem in
   * 3 Byte übergeben werden kann. */
  *p++ = 18;
  *p++ = 9;

  for (uint8_t i = 0; i < 3; i++)
  {
    uint8_t byte = command[i];
    for (uint8_t mask = 0x80; mask; mask >>= 1)
    {
      if (byte & mask)
      {
        *p++ = 9;
        *p++ = 18;
      }
      else
      {
        *p++ = 18;
        *p++ = 9;
      }
    }
  }
  *p = 7;                       // sync

  ASK_TX_ENABLE;

  for (uint8_t ii = cnt; ii > 0; ii--)
  {
    wdt_kick();
    uint8_t ask_trigger_level = 1;
    for (uint8_t i = 0; i < 51; i++)
    {
      ASK_TX_TRIGGER(ask_trigger_level ^= 1, code[i] * delay);
    }
    ASK_TX_TRIGGER(0, 64 * delay);
  }

  ASK_TX_DISABLE;
}
#endif /* PROTO_OASEFMMASTER_SUPPORT */

#ifdef PROTO_FA20RF_SUPPORT
#define FA20RF_SBIT_HITIME     6080     // 8100us
#define FA20RF_SBIT_LOTIME     610      //  960us
#define FA20RF_ONE_HITIME      640      //  740us
#define FA20RF_ONE_LOTIME      1965     // 2800us
#define FA20RF_ZERO_HITIME     640      //  740us
#define FA20RF_ZERO_LOTIME     945      // 1400us

void
ask_fa20rf_send(uint32_t device, uint8_t repeat)
{
  uint16_t code[50];
  uint16_t *p = code;

  *p++ = FA20RF_SBIT_HITIME;
  *p++ = FA20RF_SBIT_LOTIME;

  for (uint32_t mask = 0x800000; mask; mask >>= 1)
  {
    *p++ = (device & mask) ? FA20RF_ONE_HITIME : FA20RF_ZERO_HITIME;
    *p++ = (device & mask) ? FA20RF_ONE_LOTIME : FA20RF_ZERO_LOTIME;
  }

  ASK_TX_ENABLE;

  while (repeat--)
  {
    wdt_kick();
    uint8_t ask_trigger_level = 0;
    for (uint8_t i = 0; i < 50; i++)
    {
      ASK_TX_TRIGGER(ask_trigger_level ^= 1, code[i]);
    }
    ASK_TX_TRIGGER(1, 640);
    ASK_TX_TRIGGER(0, 12000);
  }

  ASK_TX_DISABLE;
}
#endif /* PROTO_FA20RF_SUPPORT */
