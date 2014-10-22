# periodic / scheduler config helper

mcu_has_timer_3_support() {
	# check mcu for second 16Bit timer/counter-unit number 3
	echo -e "#include <avr/io.h>\nTCNT3" | avr-gcc -mmcu=${MCU} -E -dD - | ${SED} '$!d;s/TCNT3/n/;s/^[^n].*$/y/'
}
