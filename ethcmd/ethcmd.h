/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#ifndef _ETHCMD_H
#define _ETHCMD_H

#include "ethcmd_message.h"

/* global configuration struct */
struct global_config_t {
    int verbose;                /* verbosity level */
    char *host;                 /* remote host */
    unsigned int port;          /* remote port */
    int sock;                   /* socket fd */
    int argc;                   /* remaining command line arguments */
    char **argv;
};

extern struct global_config_t cfg;

/* defaults */
#define DEFAULT_HOST "10.0.0.5"
#define DEFAULT_PORT 2847

#define BUFSIZE 1024

#define PROTOCOL_MAJOR 0x01
#define PROTOCOL_MINOR 0x00

#define CMD_NONE -1
#define CMD_OW   0x05
#define CMD_FS20 0x06

/* macros */
/* {{{ */
#define VERBOSE_PRINTF(...) do {  \
        if (cfg.verbose > 0) {    \
            printf(__VA_ARGS__);  \
        }                         \
    } while (0);

#ifdef DEBUG
#define DEBUG_PRINTF(...) do {    \
        printf("debug: ");        \
        printf(__VA_ARGS__);      \
    } while (0);
#else
#define DEBUG_PRINTF(...)
#endif
/* }}} */

/* subsystems */
#define ETHCMD_SUBSYSTEM_VERSION 0x0000

#endif
