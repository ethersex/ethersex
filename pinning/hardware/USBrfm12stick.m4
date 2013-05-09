dnl
dnl USBrfm12stick.m4
dnl
dnl Pin Configuration for 'USBrfm12stick'.  Edit it to fit your needs.
dnl Details at http://ethersex.de/index.php?title=USBrfm12
dnl

dnl This must also be D+ of the USB signal
USB_USE_INT(0) 
dnl This pin must have the 1k5 Pullup and must be on the same port as the Interupt pin
pin(USB_DPLUS, PD2) dnl INT
pin(USB_DMINUS, PD7)
pin(USB_ENABLE, PD4, OUTPUT)

RFM12_ASK_SENSE_USE_INT(1)

/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12_0, PC2, OUTPUT)

dnl RFM12_USE_PCINT(0, PB0)  dnl PCINT0

pin(STATUSLED_RFM12_TX, PD5, OUTPUT)

pin(STATUSLED_RFM12_RX, PD6, OUTPUT)

pin(STATUSLED_HB_ACT, PB1, OUTPUT)
