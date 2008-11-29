CHAIR(INPUT)
  LEG(--stack, STACK_OPENVPN, -j, ACCEPT)
  LEG(-p, tcp, --dport, 2701, -j, DROP)

POLICY(ACCEPT)
