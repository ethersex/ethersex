dnl
dnl volkszaehler_s0_server.m4
dnl Used for Udo's S0-Server for volkszaehler.org, M1284p
dnl

/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB3,OUTPUT)
pin(ENC_INT, PB2, INPUT)
