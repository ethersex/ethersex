

#define STELLA_PORT PORTD
#define STELLA_DDR DDRD
#define STELLA_OFFSET 5

#define PINS 3

extern uint8_t stella_color[];
extern uint8_t stella_fade[];

void stella_pwm_init(void);
void stella_sort(uint8_t color[]);
void stella_timer(void);
