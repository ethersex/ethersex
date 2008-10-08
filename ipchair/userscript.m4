CHAIR(INPUT)
  LEG(-d, 192.168.100.12, -p, tcp, ! --syn,-j, DROP)
  LEG(-d, 192.168.100.12, --tcp-flags, SYN:ack, ack, -j, DROP)
  LEG(-d, 192.168.100.12, --tcp-flags, SYN:ack, ack, -j, mangle)
#ifdef IPV6_SUPPORT
  LEG(-d, 192.168.100.12, -p, icmp, --icmp-type, ECHO, -j, mangle)
#else
  LEG(-p, icmp6, --icmp-type, ECHO, -j, mangle)
#endif
  LEG(-p, udp, -j, RETURN)		dnl use policy!
  LEG(-j, drop_ecmd)

POLICY(ACCEPT)


CHAIR(mangle) POLICY(ACCEPT)

CHAIR(drop_ecmd)
  LEG(-p, tcp, ! --dport, 2701, -j, RETURN)	dnl ignore packets not addressed to ecmd
POLICY(DROP)
