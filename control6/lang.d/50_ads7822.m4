################################
# ADS7822 12bit ADC Mittelwert (2^COUNT)
################################
define(`ADS7822_USED', `ifdef(`ads7822_used', `', `dnl
define(`old_divert', divnum)dnl
define(`ads7822_used')dnl
divert(globals_divert)
#ifndef ADS7822_SUPPORT
#error Please define ads7822 support
#endif

#include "hardware/adc/ads7822/ads7822.h"

static uint16_t
control6_get_ads7822(uint8_t count) {

    uint32_t sum=0;

    if (count < 1 || count > 15) count = 1;
    for ( uint16_t i=0; i < (1<<count); i++) {
        sum += get_ads();
    }
    return (sum>>count)&0xffff;
}

divert(old_divert)')')

define(`ADS7822_GET', `ADS7822_USED()control6_get_ads7822($1)')

