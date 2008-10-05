divert(-1)
define(`forloop',
		`pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')dnl
		define(`_forloop',
			`$4`'ifelse($1, `$3', ,
				`define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')dnl

define(`input_filter', `input:
_ipchair_args($@)')
define(`_ipchair_args', `if (_ipchair_arg_loop($@)')
define(`_ipchair_arg_loop', `dnl
dnl Destination IP Address
ifelse(`$1', `-d', `ipchair_dst($2) && $0(shift(shift($@)))')dnl
ifelse(`$1', `! -d', `!(ipchair_dst($2)) && $0(shift(shift($@)))')dnl
dnl Source IP Address
ifelse(`$1', `-s', `ipchair_src($2) && $0(shift(shift($@)))')dnl
ifelse(`$1', `! -s', `!(ipchair_src($2)) && $0(shift(shift($@)))')dnl
dnl Proto
ifelse(`$1', `-p', `ipchair_proto($2) && $0(shift(shift($@)))')dnl
dnl Source and Destination Ports
ifelse(`$1', `--dport', `ipchair_dport($2) && $0(shift(shift($@)))')dnl
ifelse(`$1', `! --dport', `!(ipchair_dport($2)) && $0(shift(shift($@)))')dnl
ifelse(`$1', `--sport', `ipchair_sport($2) && $0(shift(shift($@)))')dnl
ifelse(`$1', `! --sport', `!(ipchair_sport($2)) && $0(shift(shift($@)))')dnl
dnl TCP Flags
ifelse(`$1', `--tcp-flags', `ipchair_tcp_flags($2, $3) && $0(shift(shift(shift($@))))')dnl
ifelse(`$1', `! --tcp-flags', `!(ipchair_tcp_flags($2, $3)) && $0(shift(shift(shift($@))))')dnl
ifelse(`$1', `--syn', `ipchair_tcp_flags(`SYN:RST:ACK:FIN', `SYN') && $0(shift($@))')dnl
ifelse(`$1', `! --syn', `!(ipchair_tcp_flags(`SYN:RST:ACK:FIN', `SYN')) && $0(shift($@))')dnl
dnl Target
ifelse(`$1', `-j', `1) { 
  target(shift($@))undefine(`__proto')
}')dnl
')

# Yippieyah voodoo
define(`ipchair_addr', `ifelse(regexp($1, `:'), `-1', `translit(`$1', `.', `,')', indir(`regexp', `$1', `\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\)', ``0x\1,0x\2,0x\3,0x\4,0x\5,0x\6,0x\7,0x\8''))') 

define(`__paste2', `$1$2')
#######
# Tests
#######
define(`ipchair_dst', `uip_ipaddr_cmp_instant(BUF->destipaddr, ipchair_addr($1))') 
define(`ipchair_src', `uip_ipaddr_cmp_instant(BUF->srcipaddr, ipchair_addr($1))') 
define(`ipchair_proto',  `define(`__proto', translit(`$1', `a-z', `A-Z'))BUF->proto == __paste2(`UIP_PROTO_', translit(`$1', `a-z', `A-Z'))') 
define(`ipchair_dport', `__paste2(`BUF_', indir(`__proto'))->dstport == HTONS($1)') 
define(`ipchair_sport', `__paste2(`BUF_', indir(`__proto'))->srcport == HTONS($1)') 
define(`ipchair_tcp_flags', `(BUF_TCP->flags & (0 patsubst(`:'translit(`$1', `a-z', `A-Z'), `:', ` | TCP_')) == (0 patsubst(`:'translit(`$2', `a-z', `A-Z'), `:', ` | TCP_')))')
######
# Targets
######
define(`target',`dnl
ifelse(`$1', `DROP', `goto drop;')dnl
')
divert(0)dnl
input_filter(-d, 192.168.100.12, ! --syn, -j, DROP)
input_filter(-d, 192.168.100.12, --tcp-flags, SYN:ack, ack, -j, DROP)
