
get_usart_count() {
  USARTS=$(( $(echo "#include <avr/io.h>" | avr-gcc -mmcu=$MCU -C -E -dD - | sed -n 's/.* UDR\([0-9]*\).*/\1/p' | sort -n -r | sed -n 1p) + 1 ))
}

usart_choice() {
  i=0
  while [ $i -lt $USARTS ]; do
    echo "$i  $1_USART_$i"
    i=$(( $i + 1))
  done
}

usart_process_choice() {
  i=0
  while [ $i -lt $USARTS ]; do
  	this_usart=$(eval "echo \$${1}_USART_${i}")
    if [ "$this_usart" = y ]; then
      define_symbol "$1_USE_USART" $i
      break
    fi
    i=$(( $i + 1))
  done
}

usart_count_used() {
  USARTS_USED=0

  # DEBUG and ECMD share the same channel.
  # Possible combinations of Debug, Syslog and ECMD.
  # Result is the number of USARTS in use.
  #
  #      Sys- De-
  # ECMD log  bug Result
  #  n    n    n   0
  #  n    n    y   1
  #  n    y    n   0 *1
  #  n    y    y   0
  #  y    n    n   1
  #  y    n    y   1
  #  y    y    n   1 *2
  #  y    y    y   1
  #
  # *1 Syslog without debug should not happen, but can be achieved.
  # *2 Again invalid syslog/debug combination, ECMD is valid.

  # The two following if statements are position dependent!
  if [ "$DEBUG" = y ] || [ "$ECMD_SERIAL_USART_SUPPORT" = y ] ; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$ECMD_SERIAL_USART_SUPPORT" != y ] && [ "$DEBUG_USE_SYSLOG" = y ] && [ "$DEBUG" = y ] ; then
    USARTS_USED=$(($USARTS_USED - 1))
  fi

  if [ "$DC3840_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$DMX_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$ELTAKOMS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$FNORDLICHT_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$MCUF_SERIAL_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$MODBUS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$MSR1_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$NMEA_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$PROJECTOR_SANYO_Z700_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$SERIAL_LINE_LOG_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$SGC_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$SMS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$TO1_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$USART_SPI_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$YPORT_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$ZBUS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
}

get_usart_count
usart_count_used
