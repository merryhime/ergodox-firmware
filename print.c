/* Very basic print functions, intended to be used with usb_debug_only.c
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Version 1.0: Initial Release

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "print.h"

void print_P(const char *s)
{
	char c;

	while (1) {
		c = pgm_read_byte(s++);
		if (!c) break;
		if (c == '\n') usb_debug_putchar('\r');
		usb_debug_putchar(c);
	}
}

void pbin(uint8_t c)
{
	usb_debug_putchar(((c >> 7) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 6) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 5) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 4) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 3) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 2) & 1) ? '1' : '0');
	usb_debug_putchar(((c >> 1) & 1) ? '1' : '0');
	usb_debug_putchar(((c     ) & 1) ? '1' : '0');
}

void phex1(uint8_t c)
{
	usb_debug_putchar(c + ((c < 10) ? '0' : 'A' - 10));
}

void phex(uint8_t c)
{
	phex1(c >> 4);
	phex1(c & 15);
}

void phex16(uint16_t i)
{
	phex(i >> 8);
	phex(i);
}

void pdec16(uint16_t i)
{
	if (((i/10000) % 10) != 0) goto p4;
	if (((i/1000) % 10) != 0) goto p3;
	if (((i/100) % 10) != 0) goto p2;
	if (((i/10) % 10) != 0) goto p1;
	goto p0;
	p4: usb_debug_putchar(((i/10000) % 10) + '0');
	p3: usb_debug_putchar(((i/1000) % 10) + '0');
	p2: usb_debug_putchar(((i/100) % 10) + '0');
	p1: usb_debug_putchar(((i/10) % 10) + '0');
	p0: usb_debug_putchar(((i) % 10) + '0');
}

void pdec8(uint8_t i)
{
	if (((i/100) % 10) != 0) goto p2;
	if (((i/10) % 10) != 0) goto p1;
	goto p0;
	p2: usb_debug_putchar(((i/100) % 10) + '0');
	p1: usb_debug_putchar(((i/10) % 10) + '0');
	p0: usb_debug_putchar(((i) % 10) + '0');
}




