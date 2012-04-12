################################
# ADC
################################
define(`ADC_USED', `ifdef(`adc_used', `', `dnl
define(`old_divert', divnum)dnl
define(`adc_used')dnl
divert(globals_divert)
#ifndef ADC_SUPPORT
#error Please define adc support
#endif

#include "hardware/adc/adc.h"

static uint16_t
control6_get_adc(uint8_t sensorchannel){
  return adc_get(sensorchannel);
}

divert(old_divert)')')

define(`ADC_GET', `ADC_USED()control6_get_adc($1)')

