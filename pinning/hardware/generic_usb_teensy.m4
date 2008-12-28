dnl
dnl generic-usb-teensy.m4.m4
dnl
dnl Pin Configuration for 'generic-usb-teensy.m4'.  Edit it to fit your needs.
dnl

dnl This must also be D+ of the USB signal
USB_USE_INT(1) 
dnl This pin must have the 1k5 Pullup and must be on the same port as the Interupt pin
pin(USB_DMINUS, PD5)
pin(USB_DPLUS, PD3)

pin(GENERIC_TX_PIN, PD6)
pin(GENERIC_RX_PIN, PD7)

/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12, SPI_CS_HARDWARE)

ifdef(`conf_RFM12', `
/* port the LEDS for rfm12 txrx attached to */
pin(RFM12_TX_PIN, GENERIC_TX_PIN)
pin(RFM12_RX_PIN, GENERIC_TX_PIN)

RFM12_USE_INT(0)
')
