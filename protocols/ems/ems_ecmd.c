/*
 * Copyright (c) 2011 by Danny Baumann <dannybaumann@web.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3)
 * as published by the Free Software Foundation.
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

#include <string.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "protocols/ems/ems.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef EMS_DEBUG_STATS
int16_t parse_cmd_ems_stats(char *cmd, char *output, uint16_t len)
{
  struct ems_stats *stats = &ems_stats_buffer;

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != 23) {  /* indicator flag: real invocation:  0 */
    cmd[0] = 23;       /*                 continuing call: 23 */
    cmd[1] = 0;        /* counter for output lines */
  } else {
    cmd[1]++;          /* iterate to next output line */
  }

  enum {
    CNT_TOTALBYTES = 0,
    CNT_GOODBYTES,
    CNT_DROPPEDBYTES,
    CNT_GOODPACKETS,
    CNT_BADPACKETS,
    CNT_ONEBYTEPACKETS,
    CNT_ACKNACKPACKETS,
    CNT_OVERFLOW,
    CNT_MAXFILL
  };

  switch (cmd[1]) {
    case CNT_TOTALBYTES:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Bytes total:%lu"), stats->total_bytes));
    case CNT_GOODBYTES:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Bytes good:%lu"), stats->good_bytes));
    case CNT_DROPPEDBYTES:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Bytes dropped:%lu"), stats->dropped_bytes));
    case CNT_GOODPACKETS:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Packets good:%lu"), stats->good_packets));
    case CNT_BADPACKETS:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Packets bad:%lu"), stats->bad_packets));
    case CNT_ONEBYTEPACKETS:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Packets 1byte:%lu %lu"),
        stats->onebyte_packets, stats->onebyte_own_packets));
    case CNT_ACKNACKPACKETS:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Packets ack:%lu nack:%lu"),
        stats->onebyte_ack_packets, stats->onebyte_nack_packets));
    case CNT_OVERFLOW:
      return ECMD_AGAIN(snprintf_P(output, len, PSTR("Overflow:%lu"), stats->buffer_overflow));
    case CNT_MAXFILL:
      return ECMD_FINAL(snprintf_P(output, len, PSTR("Max fill:%u"), stats->max_fill));
  }

  return ECMD_FINAL_OK;	/* never reached */
}
#endif

/*
  -- Ethersex META --
  block([[EMS]] commands)
  ecmd_ifdef(EMS_DEBUG_STATS)
    ecmd_feature(ems_stats, "ems stats", , Report statistic counters)
  ecmd_endif()
*/
