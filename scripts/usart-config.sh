# get USART count and usage

. scripts/osdefaults.sh

get_usart_count() {
  USARTS=$(echo "#include <avr/io.h>" | avr-gcc -mmcu=$MCU -E -dD - |\
    ${AWK} "BEGIN {numusart=0};\
     /.* UDR[0-9]* .*\$/ { num=substr(\$2, 4);\
     num++;\
     if ( num > numusart ) { numusart=num } };\
     END { print numusart }")
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
  # Result is the number of USARTS in use.
  #
  # Allow parallel usage of debug and ecmd on one USART
  if [ "$DEBUG_SERIAL_USART_SUPPORT" = y -a "$ECMD_SERIAL_USART_SUPPORT" = y ] ; then
    if [ "$DEBUG_USE_USART" = "$ECMD_SERIAL_USART_USE_USART" ] ; then
      USARTS_USED=$(($USARTS_USED + 1))
    else
      USARTS_USED=$(($USARTS_USED + 2))
    fi
  elif [ "$DEBUG_SERIAL_USART_SUPPORT" = y -o "$ECMD_SERIAL_USART_SUPPORT" = y ] ; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$BSBPORT_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
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
  if [ "$EMS_SUPPORT" = y ]; then
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
