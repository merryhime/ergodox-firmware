void led_init(void);
void led_tick(void);

// which: bit 0 == Teensy, bit 1-3 == LEDa-c
// flash overrides soft overrides on
void led_on(uint8_t which);
void led_soft(uint8_t which);
void led_flash(uint8_t which);

uint8_t led_geton(void);
uint8_t led_getsoft(void);
uint8_t led_getflash(void);