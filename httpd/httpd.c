/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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
 */

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "httpd.h"
#include "base64.h"
#include "../eeprom.h"
#include "../ecmd_parser/ecmd.h"
#include "../vfs/vfs.h"


#ifdef DEBUG_HTTPD
# include "../debug.h"
# define printf        debug_printf
#else
# define printf(...)   ((void)0)
#endif


void
httpd_init(void)
{
    uip_listen(HTONS(HTTPD_PORT), httpd_main);
    uip_listen(HTONS(HTTPD_ALTERNATE_PORT), httpd_main);
}



void
httpd_cleanup (void)
{
#ifdef VFS_SUPPORT
    if (STATE->handler == httpd_handle_vfs && STATE->u.vfs.fd) {
	printf("httpd: cleaning left-over vfs-handle at %p.\n",
	       STATE->u.vfs.fd);

	vfs_close (STATE->u.vfs.fd);
	STATE->u.vfs.fd = NULL;
    }
#endif	/* VFS_SUPPORT */
}


static void
httpd_handle_input (void)
{
    if (uip_len < 6) {
	printf ("httpd: received request to short (%d bytes).\n", uip_len);
	STATE->handler = httpd_handle_400;
	return;
    }

    if (strncasecmp_P (uip_appdata, PSTR ("GET /"), 5)) {
	printf ("httpd: received request is not GET.\n");
	STATE->handler = httpd_handle_400;
	return;
    }

    char *filename = uip_appdata + 5; /* beyond slash */
    char *ptr = strchr (filename, ' ');

    if (ptr == NULL) {
	printf ("httpd: space after filename not found.\n");
	STATE->handler = httpd_handle_400;
	return;
    }

    *ptr = 0;			/* Terminate filename. */

    /*
     * Successfully parsed the GET request,
     * possibly check authentication.
     */


#ifdef HTTPD_AUTH_SUPPORT
    ptr ++;			/* Increment pointer to the end of
				   the GET */
    ptr = strstr_P(ptr, PSTR("Authorization: "));

    if (ptr == NULL) {
	printf ("Authorization-header not found.\n");
	goto auth_failed;
    }
    ptr += 15;			/* Skip `Authorization: ' header. */

    if (strncmp_P(ptr, PSTR("Basic "), 6)) {
	printf ("auth: method is not basic.\n");
	goto auth_failed;
    }
    ptr += 6;			/* Skip `Basic ' string. */

    char *nl = strchr (ptr, '\n');
    if (nl == NULL) {
	printf ("auth: newline not found.\n");
	goto auth_failed;
    }

    *nl = 0;			/* Zero-terminate BASE64-string. */

    base64_str_decode (ptr);
    printf ("auth: decoded auth string: '%s'.\n", ptr);

    if (strncmp_P (ptr, PSTR(CONF_HTTPD_USERNAME ":"),
		   strlen (CONF_HTTPD_USERNAME ":")) != 0) {
	printf ("auth: username mismatch!\n");
	goto auth_failed;
    }

    char pwd[sizeof(((struct eeprom_config_t *) 0)->httpd_auth_password) + 1];
    eeprom_restore(httpd_auth_password, pwd, sizeof(pwd));

    if (strncmp(pwd, ptr + strlen(CONF_HTTPD_USERNAME ":"), strlen(pwd))) {
	printf ("auth: wrong passphrase, %s != %s.\n",
		pwd, ptr + strlen(CONF_HTTPD_USERNAME ":"));
      auth_failed:
	STATE->handler = httpd_handle_401;
	return;
    }

#endif	/* HTTPD_AUTH_SUPPORT */

    /*
     * Authentication is okay, now fulfill request for file
     * refered to in filename.
     */
    if (*filename == 0)		/* No filename, override -> index */
	strcpy_P(filename, PSTR(HTTPD_INDEX));

    uint8_t offset = strlen_P(PSTR(ECMD_INDEX "?"));
    if (strncmp_P (filename, PSTR(ECMD_INDEX "?"), offset) == 0) {
	httpd_handle_ecmd_setup (filename + offset);
	return;
    }

#ifdef VFS_SUPPORT
    /* Keep content-type identifing char. */
    STATE->u.vfs.content_type = *filename;

    STATE->u.vfs.fd = vfs_open (filename);
    if (STATE->u.vfs.fd) {
#ifdef VFS_TEENSY
      printf ("httpd: VFS got it, serving %d bytes!\n",
	      vfs_size (STATE->u.vfs.fd));
#else
      printf ("httpd: VFS got it, serving %ld bytes!\n",
	      vfs_size (STATE->u.vfs.fd));
#endif
      STATE->handler = httpd_handle_vfs;
      return;
    }
#endif	/* VFS_SUPPORT */


    /* Fallback, send 404. */
    STATE->handler = httpd_handle_404;
}



void
httpd_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	httpd_cleanup ();
	printf ("httpd: connection aborted\n");
    }

    if (uip_closed()) {
	httpd_cleanup ();
	printf ("httpd: connection closed\n");
    }

    if (uip_connected()) {
	printf ("httpd: new connection\n");

	/* initialize struct */
	STATE->handler = NULL;
	STATE->header_acked = 0;
	STATE->eof = 0;
    }

    if (uip_newdata()) {
	printf ("httpd: new data\n");
	httpd_handle_input ();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected()) {

	/* Call associated handler, if set already. */
	if (STATE->handler)
	    STATE->handler ();
    }

}
