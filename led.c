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
#include <avr/pgmspace.h>
#include "led.h"
#include "time.h"

// D6 is Teensy LED
// B5,6,7 are LEDa,b,c
// States: Floating == LED off, High == LED on

// state bit 0 == Teensy
// state bit 1-3 == LEDa-c

void led_init(void) {
	
}

static uint8_t soft = 0;
static uint8_t on = 0;
static uint8_t flash = 0;

static void led_activate(uint8_t which) {
	if (which & 1) {
		DDRD  |= 0b01000000;
	} else {
		DDRD  &= 0b10111111;
	}

	which &= 0b1110;
	which <<= 4;

	DDRB  = (DDRB  & 0b00011111) | which;
	PORTB = (PORTB & 0b00011111) | which;
}

void led_soft(uint8_t which) {
	led_activate(which | on);

	if ((soft ^ which) & 0b0001) {
		if (which & 1) {
			PRR1  &= 0b11101111; // Disable Power Reduction: PRTIM4 = 0
			TCCR4A = 0b00000000;
			TCCR4B = 0b10001010; // PWM4X = 1, CS4 = 0b1010
			TCCR4C = 0b00000101; // COM4D = 0b01, PWM4D = 1
			TCCR4D = 0b00000000; // WGM4 = 0b00
			TCCR4E = 0b00010000; // OC4OE4 = 1 (Output enable for PD6)
		} else {
			TCCR4B = 0;
			PRR1  |= 0b00010000; // Enable Power Reduction: PRTIM4 = 1
		}
	}
	if ((soft ^ which) & 0b1110) {		
		if ((which & 0b1110) && !(soft & 0b1110)) {
			PRR0  &= 0b11110111; // Disable Power Reduction: PRTIM1 = 0
			TCCR1B = 0b00001010; // WGM1[3..2] = 0b01, CS1 = 0b010
		}
		if (!(which & 0b1110)) {
			TCCR1B = 0;
			PRR0  |= 0b00001000; // Enable Power Reduction: PRTIM1 = 1
		} else {
			uint8_t a = 0b00000001; // WGM1[1..0] = 0b01
			if (which & 0b0010) a |= 0b10000000; // COM1A 
			if (which & 0b0100) a |= 0b00100000; // COM1B
			if (which & 0b1000) a |= 0b00001000; // COM1C
			TCCR1A = a;
		}
	}

	soft = which;
}

void led_on(uint8_t which) {
	led_activate(which | soft);

	if (which & 1) {
		PORTD |= 0b01000000;
	} else {
		PORTD &= 0b10111111;
	}

	on = which;
}

void led_flash(uint8_t which) {
	led_activate(on | soft);

	flash = which;
}

uint8_t led_geton(void) {
	return on;
}

uint8_t led_getsoft(void) {
	return soft;
}

uint8_t led_getflash(void) {
	return flash;
}

void led_tick(void) {
	if (soft) {
		uint8_t v = (milliseconds>>3);
		if (v & 0x80) v = ~v;
		if (v >= 119) v = 255;
		else {
			v = (uint16_t)v*(uint16_t)v*4/225;
		}
		if (soft & 0b0001) OCR4D = v;
		if (soft & 0b1110) OCR1A = OCR1B = OCR1C = v;
	}

	if (flash) {
		if (milliseconds & (1 << 9)) 
			led_activate(on | soft | flash);
		else
			led_activate((on | soft) & ~flash);
	}
}