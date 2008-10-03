
get_usart_count() {
  USARTS=$(( $(echo "#include <avr/io.h>" | avr-gcc -mmcu=$MCU -C -E -dD - | sed -n 's/.* UDR\([0-9]\+\).*/\1/p' | sort -n -r | sed -n 1p) + 1 ))
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

  if [ "$MODBUS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$YPORT_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$ZBUS_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$MCUF_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
  if [ "$ECMD_SERIAL_USART_SUPPORT" = y ]; then
    USARTS_USED=$(($USARTS_USED + 1))
  fi
}

get_usart_count
usart_count_used
