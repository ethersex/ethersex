typedef struct {
	unsigned char rufnummer[20];
	uint8_t text_len;
	unsigned char text[120];	/* text speicher und erst vor dem verschicken bytes2string */
	void (*sms_send_err_calback)();
} sms;

void callback(void);
void ben_test(void);

uint8_t sms_send(uint8_t *rufnummer, unsigned char *text, void (*ptr)(), uint8_t nr_is_encoded);
void sms_transmit_handler(void);

void sms_uart_init(void);

void sms_periodic_timeout(void);
