/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "appsample.h"

void
app_sample_init(void)
{
  APPSAMPLEDEBUG ("init\n");
}

void
app_sample_periodic(void)
{
  APPSAMPLEDEBUG ("periodic\n");
}

/*
  -- Ethersex META --
  header(services/appsample/appsample.h)
  ecmd_ifdef(APP_SAMPLE_INIT_AUTOSTART)
    init(app_sample_init)
  ecmd_endif()
  ecmd_ifdef(APP_SAMPLE_PERIODIC_AUTOSTART)
    timer(100,app_sample_periodic())
  ecmd_endif()
*/
