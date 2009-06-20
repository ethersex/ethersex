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

divert(old_divert)')')

define(`IPADDR', `ifelse(regexp($1, `:'), `-1', `ip4addr_expand(translit(`$1', `.', `,'))', 
	`ip6addr_expand(regexp(`$1', `\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\)',`0x\1,0x\2,0x\3,0x\4,0x\5,0x\6,0x\7,0x\8')) ') ')

define(`ip4addr_expand', `uip_ipaddr_t ip; uip_ipaddr(&ip, $1, $2, $3, $4)')
define(`ip6addr_expand', `uip_ipaddr_t ip; uip_ip6addr(&ip, $1, $2, $3, $4, $5, $6, $7, $8)')

define(`ESEND', `do {IPADDR($1); ecmd_sender_send_command(&ip, PSTR($2), NULL); } while(0)')

