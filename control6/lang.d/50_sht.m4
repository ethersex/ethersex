################################
# SHT
################################
define(`SHT_USED', `ifdef(`sht_used', `', `dnl
define(`old_divert', divnum)dnl
define(`sht_used')dnl
divert(globals_divert)
#ifndef SHT_SUPPORT
#error Please define sht support
#endif

#include "hardware/sht/sht.h"

static uint16_t
control6_get_sht(uint8_t choice){

    uint16_t sht_temp=0;
    uint16_t sht_humid=0;

    int16_t ret = sht_get(&sht_temp,&sht_humid);

    switch (choice) {
        case 0:
                if (ret == SHT_OK)
                    return sht_convert_temp(&sht_temp);
                else
                    return 1000;
                break;
        default:
                if (ret == SHT_OK)
                    return sht_convert_humid(&sht_temp,&sht_humid);
                else
                    return 9999;
                break;
    }
}

divert(old_divert)')')

define(`SHT_GET_HUMID', `SHT_USED()control6_get_sht(1)')
define(`SHT_GET_TEMP', `SHT_USED()control6_get_sht(0)')

