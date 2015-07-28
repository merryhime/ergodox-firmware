#ifndef print_h__
#define print_h__

#include <avr/pgmspace.h>
#include "usb_keyboard_debug.h"

// this macro allows you to write print("some text") and
// the string is automatically placed into flash memory :)
#define print(s) print_P(PSTR(s))
#define pchar(c) usb_debug_putchar(c)

void print_P(const char *s);
void phex(uint8_t c);
void phex16(uint16_t i);
void pbin(uint8_t i);
void pdec16(uint16_t i);
void pdec8(uint8_t i);

#endif
