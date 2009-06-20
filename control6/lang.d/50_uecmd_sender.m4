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

#include "protocols/ecmd/via_tcp/ecmd_state.h"

divert(old_divert)')')

define(`UESEND', `UECMD_SENDER_USED()do {IPADDR($1);uecmd_sender_send_command(&ip, PSTR($2), NULL); } while(0)')

define(`UESENDGET', `INTHREAD(`', `DIE(`Can use UESENDGET only in a THREAD')')UECMD_SENDER_USED(){IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1; 
uecmd_sender_send_command(&ip, PSTR($2), uecmd_callback'action_thread_ident`); 
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);' }
ifdef(`uecmd_callback_defined'action_thread_ident, `', `
define(`uecmd_callback_defined'action_thread_ident, 1)
define(`old_divert', divnum)dnl
divert(globals_divert)dnl
uint8_t uecmd_callback_blocking'action_thread_ident`;
uint8_t uecmd_callback_buffer'action_thread_ident`[ECMD_INPUTBUF_LENGTH];
uint8_t uecmd_callback_buffer_len'action_thread_ident`;

void uecmd_callback'action_thread_ident`(char *text, uint8_t len) {
  uecmd_callback_blocking'action_thread_ident` = 0;
  uecmd_callback_buffer_len'action_thread_ident` = len;
  if (text) {
    memset(uecmd_callback_buffer'action_thread_ident`, 0, ECMD_INPUTBUF_LENGTH);
    memcpy(uecmd_callback_buffer'action_thread_ident`, text, 
           (ECMD_INPUTBUF_LENGTH - 1) < len ? ECMD_INPUTBUF_LENGTH - 1 : len);
    
  }
}')

divert(old_divert)')')

define(`UESENDGET_BUFFER', `INTHREAD(`', `DIE(`Can use USENDGET_BUFFER only in a THREAD')')`uecmd_callback_buffer'action_thread_ident')
define(`UESENDGET_BUFFER_LEN', `INTHREAD(`', `DIE(`Can use USENDGET_BUFFER_LEN only in a THREAD')')`uecmd_callback_buffer_len'action_thread_ident')

