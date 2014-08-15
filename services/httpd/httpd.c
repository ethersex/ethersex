/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "httpd.h"
#include "base64.h"
#include "core/eeprom.h"
#include "core/vfs/vfs.h"
#include "services/pam/pam_prototypes.h"

#ifdef DEBUG_HTTPD
#include "core/debug.h"
#define printf(a...)  debug_printf(a)
#else
#define printf(...)   ((void)0)
#endif


void
httpd_init(void)
{
  uip_listen(HTONS(HTTPD_PORT), httpd_main);
  uip_listen(HTONS(HTTPD_ALTERNATE_PORT), httpd_main);
}



void
httpd_cleanup(void)
{
#ifdef VFS_SUPPORT
  if (STATE->handler == httpd_handle_vfs && STATE->u.vfs.fd)
  {
    printf("httpd: cleaning left-over vfs-handle at %p.\n", STATE->u.vfs.fd);

    vfs_close(STATE->u.vfs.fd);
    STATE->u.vfs.fd = NULL;
  }
#endif /* VFS_SUPPORT */

#ifdef HTTP_SD_DIR_SUPPORT
  if (STATE->handler == httpd_handle_sd_dir && STATE->u.dir.handle)
  {
    fat_close_dir(STATE->u.dir.handle);
    STATE->u.dir.handle = NULL;
  }
#endif /* HTTP_SD_DIR_SUPPORT */

#ifdef HTTPD_SOAP_SUPPORT
  if (STATE->handler == httpd_handle_soap)
    soap_deallocate_context(&STATE->u.soap);
#endif /* HTTPD_SOAP_SUPPORT */

  STATE->handler = NULL;
}


static void
httpd_handle_input(void)
{
  char *ptr = (char *) uip_appdata;

#ifdef HTTPD_AUTH_SUPPORT
  char *start_ptr;
  if (STATE->header_reparse)
  {
    printf("reparse next part of the header\n");
    goto start_auth;
  }
#endif /* HTTPD_AUTH_SUPPORT */

  if (uip_len < 10)
  {
    printf("httpd: received request to short (%d bytes).\n", uip_len);
    STATE->handler = httpd_handle_400;
    return;
  }

#ifdef HTTPD_SOAP_SUPPORT
  if (strncasecmp_P(uip_appdata, PSTR("POST /soap"), 10) == 0)
  {
    soap_initialize_context(&STATE->u.soap);
    STATE->handler = httpd_handle_soap;
    return;
  }
#endif /* HTTPD_SOAP_SUPPORT */

  if (strncasecmp_P(uip_appdata, PSTR("GET /"), 5))
  {
    printf("httpd: received request is not GET.\n");
    STATE->handler = httpd_handle_400;
    return;
  }

  char *filename = (char *)uip_appdata + 5;     /* beyond slash */
  ptr = strchr(filename, ' ');

  if (ptr == NULL)
  {
    printf("httpd: space after filename not found.\n");
    STATE->handler = httpd_handle_400;
    return;
  }

  *ptr = 0;                     /* Terminate filename. */

  /*
   * Successfully parsed the GET request,
   * possibly check authentication.
   */


#ifdef HTTPD_AUTH_SUPPORT
  ptr++;                        /* Increment pointer to the end of
                                 * the GET */
start_auth:
  start_ptr = ptr;
  ptr = strstr_P(ptr, PSTR("Authorization: "));

  if (ptr == NULL)
  {
    if (strstr_P(start_ptr, PSTR("\r\n\r\n")))
    {
      printf("Authorization-header not found.\n");
      printf("%s\n", start_ptr);
      goto auth_failed;
    }
    else
    {
      ptr = start_ptr;
      /* Skip all Lines before the last one */
      while (1)
      {
        ptr = strstr_P(start_ptr, PSTR("\r\n"));
        if (ptr)
          start_ptr = ptr + 2;
        else
          break;
      }
      if (!strncmp(start_ptr, PSTR("Authorization: "), strlen(start_ptr)))
      {
        printf("Authorization header is split over two packages, damn");
        printf("%s\n", start_ptr);
        goto auth_failed;
      }
      else
      {
        STATE->header_reparse = 1;
        goto after_auth;
      }
    }
  }
  ptr += 15;                    /* Skip `Authorization: ' header. */

  if (strncmp_P(ptr, PSTR("Basic "), 6))
  {
    printf("auth: method is not basic.\n");
    goto auth_failed;
  }
  ptr += 6;                     /* Skip `Basic ' string. */

  char *nl = strchr(ptr, '\n');
  if (nl == NULL)
  {
    printf("auth: newline not found.\n");
    goto auth_failed;
  }

  *nl = 0;                      /* Zero-terminate BASE64-string. */

  base64_str_decode(ptr);
  printf("auth: decoded auth string: '%s'.\n", ptr);

  char *password;

  if ((password = strchr(ptr, ':')) == NULL)
  {
    printf("auth: didn't find colon!\n");
  auth_failed:
    httpd_cleanup();
    STATE->handler = httpd_handle_401;
    STATE->header_reparse = 0;
    return;
  }
  *password = 0;
  password++;

  /* Do the PAM authentification */
  pam_auth(ptr, password, &STATE->auth_state);

  if (STATE->header_reparse)
  {
    STATE->header_reparse = 0;
    return;                     /* We musn't open the file once again */
  }

after_auth:
  /* Dummy statement, to keep GCC happy, if HTTP_SD_DIR_SUPPORT is enabled
   * and thus the uint8_t definition the next statement in code. */
  (void) 0;

#endif /* HTTPD_AUTH_SUPPORT */

  /*
   * Authentication is okay, now fulfill request for file
   * refered to in filename.
   */
#ifndef HTTP_SD_DIR_SUPPORT
  if (*filename == 0)           /* No filename, override -> index */
    strcpy_P(filename, PSTR(HTTPD_INDEX));
#endif


#ifdef HTTP_FAVICON_SUPPORT
  if (strcmp_P(filename, PSTR("favicon.ico")) == 0)
    strcpy_P(filename, PSTR("If.ico"));
#endif



#ifdef ECMD_PARSER_SUPPORT
  uint8_t offset = strlen_P(PSTR(ECMD_INDEX "?"));
  if (strncmp_P(filename, PSTR(ECMD_INDEX "?"), offset) == 0)
  {
    httpd_handle_ecmd_setup(filename + offset);
    return;
  }
#endif /* ECMD_PARSER_SUPPORT */

#ifdef VFS_SUPPORT
  /* Keep content-type identifing char. */
  /* filename instead starts after last directory slash. */
  char *slash = strrchr(filename, '/');
  if (slash != NULL)
  {
    STATE->u.vfs.content_type = *(char *) (slash + 1);
  }
  else
  {
    STATE->u.vfs.content_type = *filename;
  }

  STATE->u.vfs.fd = vfs_open(filename);
  if (STATE->u.vfs.fd)
  {
    STATE->handler = httpd_handle_vfs;
    return;
  }

  /* Now try appending the index.html document name */
  ptr = filename + strlen(filename);
#ifdef HTTP_SD_DIR_SUPPORT
  uint8_t lastchar = ptr[-1];
#endif
  if (ptr[-1] != '/')
    *(ptr++) = '/';

  strcpy_P(ptr, PSTR(HTTPD_INDEX));
  STATE->u.vfs.fd = vfs_open(filename);
  if (STATE->u.vfs.fd)
  {
    STATE->handler = httpd_handle_vfs;
    return;
  }

  if (ptr == filename)          /* Make sure not to strip initial slash. */
    ptr[0] = 0;
  else
    ptr[-1] = 0;                /* Strip index filename again,
                                 * including the last slash. */
#endif /* VFS_SUPPORT */

#ifdef HTTP_SD_DIR_SUPPORT
  if ((STATE->u.dir.handle = vfs_sd_chdir(filename - 1)))
  {
    strncpy(STATE->u.dir.dirname, filename - 1, SD_DIR_MAX_DIRNAME_LEN);
    STATE->u.dir.dirname[SD_DIR_MAX_DIRNAME_LEN - 1] = 0;
    if (lastchar != '/')
    {
      STATE->handler = httpd_handle_sd_dir_redirect;
      fat_close_dir(STATE->u.dir.handle);
    }
    else
      STATE->handler = httpd_handle_sd_dir;
    return;
  }
#endif /* HTTP_SD_DIR_SUPPORT */

  /* Fallback, send 404. */
  STATE->handler = httpd_handle_404;
}



void
httpd_main(void)
{
  if (uip_aborted() || uip_timedout())
  {
    printf("httpd: connection aborted\n");
    httpd_cleanup();
    return;
  }

  if (uip_closed())
  {
    printf("httpd: connection closed\n");
    httpd_cleanup();
    return;
  }

  if (uip_connected())
  {
    printf("httpd: new connection\n");

    /* initialize struct */
    STATE->handler = NULL;
    STATE->header_acked = 0;
    STATE->eof = 0;
    STATE->header_reparse = 0;
#ifdef HTTPD_AUTH_SUPPORT
    STATE->auth_state = PAM_UNKOWN;
#endif
  }

  if (uip_newdata() && (!STATE->handler || STATE->header_reparse))
  {
    printf("httpd: new data\n");
    httpd_handle_input();
  }

#ifdef HTTPD_AUTH_SUPPORT
  if (STATE->auth_state == PAM_DENIED && STATE->handler != httpd_handle_401)
  {
    httpd_cleanup();
    STATE->handler = httpd_handle_401;
    STATE->header_reparse = 0;
    printf("httpd: auth failed\n");
  }
  else if (STATE->auth_state == PAM_PENDING)
    return;                     /* Waiting for the PAM Layer */
#endif


  if (uip_rexmit() ||
      uip_newdata() || uip_acked() || uip_poll() || uip_connected())
  {

    /* Call associated handler, if set already. */
    if (STATE->handler && (!STATE->header_reparse))
      STATE->handler();
  }
}

/*
  -- Ethersex META --
  header(services/httpd/httpd.h)
  net_init(httpd_init)

  state_header(services/httpd/httpd_state.h)
  state_tcp(struct httpd_connection_state_t httpd)
*/
