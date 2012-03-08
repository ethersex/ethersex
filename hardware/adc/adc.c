/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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

#include <avr/io.h>

#include "adc.h"

#ifdef ADC_VOLTAGE_SUPPORT
#include "core/eeprom.h"
#endif /* ADC_VOLTAGE_SUPPORT */

#ifndef ADC_REF
#define ADC_REF 0
#endif

uint8_t last_ref;

#ifdef ADC_VOLTAGE_SUPPORT
uint16_t vref;
#endif /* ADC_VOLTAGE_SUPPORT */

void
adc_init(void)
{
  /* ADC Prescaler to 64 */
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);

  /* init reference */
  ADMUX = ADC_REF;
  last_ref = 0xff;

#ifdef ADC_VOLTAGE_SUPPORT
  eeprom_restore_int(adc_vref, &vref);
#endif
}

uint16_t
adc_get_setref(uint8_t ref, uint8_t channel)
{
  /* select reference and channel */
  ADMUX = (ref & 0xc0) | (channel & 0x1f);
  if (last_ref != ref)
  {
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    last_ref = ref;
  }

  /* Start adc conversion */
  ADCSRA |= _BV(ADSC);
  /* Wait for completion of adc */
  loop_until_bit_is_clear(ADCSRA, ADSC);

  return ADC;
}

#ifdef ADC_VOLTAGE_SUPPORT

uint16_t
adc_get_voltage_setref(uint8_t ref, uint8_t channel)
{
  return adc_raw_to_voltage(adc_get_setref(ref, channel));
}

uint16_t
adc_raw_to_voltage(uint16_t raw)
{
  return ((float) vref * (float) raw * ADC_RES_RECIEP);
}

uint16_t
adc_get_vref()
{
  return vref;
}

void
adc_set_vref(uint16_t value)
{
  vref = value;
  eeprom_save_int(adc_vref, vref);
  eeprom_update_chksum();
}

#endif /* ADC_VOLTAGE_SUPPORT */

/*
  -- Ethersex META --
  block(Analog/Digital Conversion ([[ADC]]))
  header(hardware/adc/adc.h)
  init(adc_init)
*/
