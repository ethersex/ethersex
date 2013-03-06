################################
# DHT
################################
define(`DHT_USED', `ifdef(`dht_used', `', `dnl
define(`old_divert', divnum)dnl
define(`dht_used')dnl
divert(globals_divert)
#ifndef DHT_SUPPORT
#error Please define dht support
#endif

#include "hardware/dht/dht.h"

divert(old_divert)')')

define(`DHT_HUMIDITY', `DHT_USED()dht_global.humid')
define(`DHT_TEMPERATURE', `DHT_USED()dht_global.temp')


