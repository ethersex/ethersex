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

static char* control6_uesend_printf(PGM_P format, ...)
{
  va_list args;
  uint8_t len = strlen_P(format);
  uint8_t wlen = len;

  // first round: assume output length < format string length
  char* buf = malloc(len);

  va_start(args, format);
  while (wlen == len)
  {
    if (buf == NULL) return NULL;
    wlen = vsnprintf_P(buf, len, format, args) + 1;

    // actual size requirement in wlen now -> reallocate
	buf = realloc(buf, wlen);
    if (wlen <= len)
      break;

    // need a second round, this time with actual size requirement ...
    len = wlen;
  }
  va_end(args);
  return buf;
}

divert(old_divert)')')

define(`UESEND', `UECMD_SENDER_USED()ifelse(`$#', 2,dnl
do {IPADDR($1);uecmd_sender_pgm_send_command(&ip, PSTR($2), NULL); } while(0),
do {IPADDR($1);
char* buf = control6_uesend_printf(PSTR($2), shift(shift($@)));
if (buf != 0) 
  uecmd_sender_send_command(&ip, buf, NULL); } while(0))')

define(`UESENDGET', `INTHREAD(`', `DIE(`Can use UESENDGET only in a THREAD')')UECMD_SENDER_USED()ifelse(`$#', 2,dnl
{IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1; 
uecmd_sender_pgm_send_command(&ip, PSTR($2), uecmd_callback'action_thread_ident`); 
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);' },
{IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1; 
uecmd_buffer'action_thread_ident` = control6_uesend_printf(PSTR($2), shift(shift($@)));
uecmd_sender_send_command(&ip, uecmd_buffer'action_thread_ident`, uecmd_callback'action_thread_ident`); 
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);' })
ifdef(`uecmd_callback_defined'action_thread_ident, `', `
define(`uecmd_callback_defined'action_thread_ident, 1)
define(`old_divert', divnum)dnl
divert(globals_divert)dnl
uint8_t uecmd_callback_blocking'action_thread_ident`;
uint8_t uecmd_callback_buffer'action_thread_ident`[ECMD_INPUTBUF_LENGTH];
uint8_t uecmd_callback_buffer_len'action_thread_ident`;
char* uecmd_buffer'action_thread_ident`;

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

