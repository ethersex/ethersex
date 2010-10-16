################################
# IRMP
################################
define(`IRMP_USED', `ifdef(`irmp_used', `', `dnl
define(`old_divert', divnum)dnl
define(`irmp_used')dnl
divert(globals_divert)
#ifndef IRMP_SUPPORT
#error Please define irmp support
#endif

#include "hardware/ir/irmp/irmp.h"

static irmp_data_t control6_irmp_data_rx;

define(`IRMP_PROTOCOL', `control6_irmp_data_rx.protocol')
define(`IRMP_ADDRESS', `control6_irmp_data_rx.address')
define(`IRMP_COMMAND', `control6_irmp_data_rx.command')
define(`IRMP_FLAGS', `control6_irmp_data_rx.flags')

divert(old_divert)')')

define(`IRMP_READ', `IRMP_USED()irmp_read(&control6_irmp_data_rx)')
define(`IRMP_WRITE', `IRMP_USED()irmp_write(&control6_irmp_data_rx)')

