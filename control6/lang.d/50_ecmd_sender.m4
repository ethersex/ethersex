################################
# ECMD SENDER
################################
define(`ECMD_SENDER_USED', `ifdef(`ecmd_sender_used', `', `dnl
define(`old_divert', divnum)dnl
define(`ecmd_sender_used')dnl
divert(globals_divert)
#ifndef ECMD_SENDER_SUPPORT
#error Please define emcd sender tcp support
#endif

#include "protocols/ecmd/sender/ecmd_sender_net.h"

divert(old_divert)')')

define(`IPADDR', `ifelse(regexp($1, `:'), `-1', `ip4addr_expand(translit(`$1', `.', `,'))', 
	`ip6addr_expand(regexp(`$1', `\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\)',`0x\1,0x\2,0x\3,0x\4,0x\5,0x\6,0x\7,0x\8')) ') ')

define(`ip4addr_expand', `uip_ipaddr_t ip; uip_ipaddr(&ip, $1, $2, $3, $4)')
define(`ip6addr_expand', `uip_ipaddr_t ip; uip_ip6addr(&ip, $1, $2, $3, $4, $5, $6, $7, $8)')

define(`ESEND', `ECMD_SENDER_USED()dnl
{IPADDR($1);
ifelse(`$#', 2, `ecmd_sender_send_command_P(&ip, NULL, PSTR($2))',
                `ecmd_sender_send_command_P(&ip, NULL, PSTR($2), shift(shift($@)))');}')

define(`ESENDGET', `INTHREAD(`', `DIE(`Can use ESENDGET only in a THREAD')')ECMD_SENDER_USED()dnl
ifelse(`$#', 2,dnl
{IPADDR($1);
`ecmd_callback_blocking'action_thread_ident` = 1;
ecmd_sender_send_command_P(&ip, ecmd_callback'action_thread_ident`, PSTR($2));
PT_WAIT_WHILE(pt, ecmd_callback_blocking'action_thread_ident` == 1);' },
{IPADDR($1);
`ecmd_callback_blocking'action_thread_ident` = 1;
ecmd_sender_send_command_P(&ip, ecmd_callback'action_thread_ident`, shift(shift($@)));
PT_WAIT_WHILE(pt, ecmd_callback_blocking'action_thread_ident` == 1);'})

ifdef(`ecmd_callback_defined'action_thread_ident, `', `
define(`ecmd_callback_defined'action_thread_ident, 1)
define(`old_divert', divnum)dnl
divert(globals_divert)dnl
uint8_t ecmd_callback_blocking'action_thread_ident`;
uint8_t ecmd_callback_buffer'action_thread_ident`[ECMD_INPUTBUF_LENGTH];
uint8_t ecmd_callback_buffer_len'action_thread_ident`;
char* ecmd_buffer'action_thread_ident`;

void ecmd_callback'action_thread_ident`(char *text, uint8_t len) {
  ecmd_callback_buffer_len'action_thread_ident` = len;
  if (text) {
    memset(ecmd_callback_buffer'action_thread_ident`, 0, ECMD_INPUTBUF_LENGTH);
    memcpy(ecmd_callback_buffer'action_thread_ident`, text,
           (ECMD_INPUTBUF_LENGTH - 1) < len ? ECMD_INPUTBUF_LENGTH - 1 : len);

  }
  ecmd_callback_blocking'action_thread_ident` = 0;
}')
divert(old_divert)')')

define(`ESENDGET_BUFFER', `INTHREAD(`', `DIE(`Can use ESENDGET_BUFFER only in a THREAD')')`ecmd_callback_buffer'action_thread_ident')
define(`ESENDGET_BUFFER_LEN', `INTHREAD(`', `DIE(`Can use ESENDGET_BUFFER_LEN only in a THREAD')')`ecmd_callback_buffer_len'action_thread_ident')

