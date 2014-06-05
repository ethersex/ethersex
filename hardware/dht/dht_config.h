static const char sensor_name_keller[] PROGMEM = "keller";
static const char sensor_name_bad[] PROGMEM = "bad";

dht_sensor_t dht_sensors[] = {
	{
		.port = &PORTB,
		.pin = PB3,
		.name = sensor_name_keller
	},
	{
		.port = &PORTA,
		.pin = PA3,
		.name = sensor_name_bad
	}
};

