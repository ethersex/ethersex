

#define STELLA_PORT PORTD
#define STELLA_DDR DDRD
#define STELLA_OFFSET 5

#define PINS 3

enum stella_colors
{
  STELLA_RED,
  STELLA_GREEN,
  STELLA_BLUE
};

enum stella_commands
{
  STELLA_SET_RED,
  STELLA_SET_GREEN,
  STELLA_SET_BLUE,
  STELLA_FADE_RED,
  STELLA_FADE_GREEN,
  STELLA_FADE_BLUE,
  STELLA_FLASH_RED,
  STELLA_FLASH_GREEN,
  STELLA_FLASH_BLUE,
  STELLA_SELECT_FADE_FUNC,
};

enum
{
  STELLA_FADE_NORMAL,
  STELLA_FADE_FLASHY,
  FADE_FUNC_LEN
};

extern uint8_t stella_color[];
extern uint8_t stella_fade[];

extern uint8_t stella_fade_func;

void stella_pwm_init(void);
void stella_sort(uint8_t color[]);
void stella_timer(void);
void stella_process(unsigned char *buf, uint8_t len);
