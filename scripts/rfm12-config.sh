
get_rfm12_count() {
  RFM12S=3
}

rfm12_choice() {
  i=0
  while [ $i -lt $RFM12S ]; do
    echo "$i  $1_RFM12_$i"
    i=$(( $i + 1))
  done
}

rfm12_process_choice() {
  i=0
  while [ $i -lt $RFM12S ]; do
    this_rfm12=$(eval "echo \$${1}_RFM12_${i}")
    if [ "$this_rfm12" = y ]; then
      define_symbol "$1_USE_RFM12" $i
      break
    fi
    i=$(( $i + 1))
  done
}

rfm12_count_used() {
  RFM12S_USED=0

  if [ "$RFM12_IP_SUPPORT" = "y" ]; then
    RFM12S_USED=$(($RFM12S_USED + 1))
  fi
  if [ "$RFM12_ASK_433_SUPPORT" = "y" ]; then
    RFM12S_USED=$(($RFM12S_USED + 1))
  fi
  if [ "$RFM12_ASK_868_SUPPORT" = "y" ]; then
      RFM12S_USED=$(($RFM12S_USED + 1))
  fi
}

get_rfm12_count
rfm12_count_used

