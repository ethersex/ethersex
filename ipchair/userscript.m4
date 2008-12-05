CHAIR(INPUT)

#ifdef OPENVPN_SUPPORT
dnl If OpenVPN support is enabled, drop every packet that's directly sent
dnl to the ecmd port (2701), i.e. require ecmd to be accessed via the OpenVPN
dnl connection.
  LEG(--stack, STACK_OPENVPN, -j, ACCEPT)
  LEG(-p, tcp, --dport, 2701, -j, DROP)
#endif

POLICY(ACCEPT)
