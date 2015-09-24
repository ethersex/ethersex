################################
# UECMD SENDER
################################
define(`UECMD_SENDER_USED', `ifdef(`uecmd_sender_used', `', `dnl
define(`old_divert', divnum)dnl
define(`uecmd_sender_used')dnl
divert(globals_divert)
#ifndef UECMD_SENDER_SUPPORT
#error Please define emcd sender udp support
#endif

#include "protocols/ecmd/sender/ecmd_sender_net.h"

divert(old_divert)')')

define(`UESEND', `UECMD_SENDER_USED()dnl
{IPADDR($1);
ifelse(`$#', 2, `uecmd_sender_send_command_P(&ip, NULL, PSTR($2))',
                `uecmd_sender_send_command_P(&ip, NULL, PSTR($2), shift(shift($@)))');}')

define(`UESENDGET', `INTHREAD(`', `DIE(`Can use UESENDGET only in a THREAD')')UECMD_SENDER_USED()ifelse(`$#', 2,dnl
{IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1;
uecmd_sender_send_command_P(&ip, uecmd_callback'action_thread_ident`, PSTR($2));
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);' },
{IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1;
uecmd_sender_send_command_P(&ip, uecmd_callback'action_thread_ident`, shift(shift($@)));
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);'})

ifdef(`uecmd_callback_defined'action_thread_ident, `', `
define(`uecmd_callback_defined'action_thread_ident, 1)
define(`old_divert', divnum)dnl
divert(globals_divert)dnl
uint8_t uecmd_callback_blocking'action_thread_ident`;
uint8_t uecmd_callback_buffer'action_thread_ident`[ECMD_INPUTBUF_LENGTH];
uint8_t uecmd_callback_buffer_len'action_thread_ident`;
char* uecmd_buffer'action_thread_ident`;

void uecmd_callback'action_thread_ident`(char *text, uint8_t len) {
  uecmd_callback_buffer_len'action_thread_ident` = len;
  if (text) {
    memset(uecmd_callback_buffer'action_thread_ident`, 0, ECMD_INPUTBUF_LENGTH);
    memcpy(uecmd_callback_buffer'action_thread_ident`, text,
           (ECMD_INPUTBUF_LENGTH - 1) < len ? ECMD_INPUTBUF_LENGTH - 1 : len);

  }
  uecmd_callback_blocking'action_thread_ident` = 0;
}')
divert(old_divert)')')

define(`UESENDGET_BUFFER', `INTHREAD(`', `DIE(`Can use UESENDGET_BUFFER only in a THREAD')')`uecmd_callback_buffer'action_thread_ident')
define(`UESENDGET_BUFFER_LEN', `INTHREAD(`', `DIE(`Can use UESENDGET_BUFFER_LEN only in a THREAD')')`uecmd_callback_buffer_len'action_thread_ident')

