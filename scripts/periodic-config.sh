# periodic / scheduler config helper

source scripts/osdefaults.h

# build list of supported clock ticks per second
periodic_ticks_per_second_choice() {
	# there should be at least 2000 cpu-cycles per tick
	max_clocks_per_sec=$(( ${FREQ} / 2000 ))
	# but always allow the minimum of 50 Hz for systems using
	# e.g. 32768Hz clock oscillators
	if [ $max_clocks_per_sec -lt 50 ]; then
		max_clocks_per_sec=50
	fi
	# (pure artificial) higher limit to max 2000 ticks per second
	if [ $max_clocks_per_sec -gt 2000 ]; then
		max_clocks_per_sec=2000
	fi
  	i=50
  	while [ $i -le $max_clocks_per_sec ]; do
    	echo "$i  CLOCKS_PER_SEC_$i"
    	i=$(( $i + 50))
  	done
}

mcu_has_timer_3_support() {
	# check mcu for second 16Bit timer/counter-unit number 3
	echo -e "#include <avr/io.h>\nTCNT3" | avr-gcc -mmcu=${MCU} -E -dD - | ${SED} '$!d;s/TCNT3/n/;s/^[^n].*$/y/'
}
