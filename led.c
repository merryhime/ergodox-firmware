/*
 * MerryMage ErgoDox Firmware - led.c
 *
 * Copyright (c) 2015, MerryMage
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include <avr/io.h>

// D6 is Teensy LED
// B5,6,7 are LEDa,b,c
// States: Floating == LED off, High == LED on

// state bit 0 == Teensy
// state bit 1-3 == LEDa-c

void led_set(uint8_t state) {
	if (state & 1) {
		DDRD  |= 0b01000000;
		PORTD |= 0b01000000;
	} else {
		DDRD  &= 0b10111111;
		PORTD &= 0b10111111;
	}

	state &= 0b1110;
	state <<= 4;

	DDRB  = (DDRB  & 0b00011111) | state;
	PORTB = (PORTB & 0b00011111) | state;
}

uint8_t led_get() {
	return ((DDRB & 0b11100000) >> 4) | ((DDRD & 0b01000000) >> 6);
}