/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef ECMD_STATE_H
#define ECMD_STATE_H

#define ECMD_INPUTBUF_LENGTH  50
#define ECMD_OUTPUTBUF_LENGTH 50

struct ecmd_connection_state_t {
    char inbuf[ECMD_INPUTBUF_LENGTH];
    uint8_t in_len;
    char outbuf[ECMD_OUTPUTBUF_LENGTH];
    uint8_t out_len;
    uint8_t parse_again;
#ifdef ECMD_PAM_SUPPORT
    uint8_t pam_state;
#endif
    uint8_t close_requested;
};

#endif /* ECMD_STATE_H */
