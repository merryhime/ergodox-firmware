/*
 * MerryMage ErgoDox Firmware
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
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "usb_keyboard_debug.h"
#include "print.h"
#include "mcp23018.h"
#include "time.h"

uint8_t matrixscan[14];
uint8_t state[14] = {0, 0, 0, 0, 0, 
                     0, 0, 0, 0, 0, 
                     0, 0, 0, 0};

// Manually unrolled 0..13 loop.
#define do14(OP) \
	do { const uint8_t i = 0; OP; } while(0); \
	do { const uint8_t i = 1; OP; } while(0); \
	do { const uint8_t i = 2; OP; } while(0); \
	do { const uint8_t i = 3; OP; } while(0); \
	do { const uint8_t i = 4; OP; } while(0); \
	do { const uint8_t i = 5; OP; } while(0); \
	do { const uint8_t i = 6; OP; } while(0); \
	do { const uint8_t i = 7; OP; } while(0); \
	do { const uint8_t i = 8; OP; } while(0); \
	do { const uint8_t i = 9; OP; } while(0); \
	do { const uint8_t i = 10; OP; } while(0); \
	do { const uint8_t i = 11; OP; } while(0); \
	do { const uint8_t i = 12; OP; } while(0); \
	do { const uint8_t i = 13; OP; } while(0)

#define RIGHT_SCANLINE(COLno, COLPINx, COLPINy)    \
	do {                                           \
		DDR##COLPINx |= (1<<COLPINy);              \
		_delay_loop_1(128);                        \
		uint8_t f = ~PINF;                         \
		f = (f & 0b11) | ((f & 0b011110000) >> 2); \
		matrixscan[COLno] = f;                     \
		DDR##COLPINx &= ~(1<<COLPINy);             \
	} while(0);

uint8_t layout[6][14] = {
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0}, 
	{0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,   0, 0, 0, 0, 0, 0, 0}, 
	{0, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L,   0, 0, 0, 0, 0, 0, 0}, 
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0}, 
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0}, 
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0}
};

void change(uint8_t row, uint8_t col, uint8_t newstate) {
	//print("SW");pdec8(row);print(":");pdec8(col);
	//if (newstate) print(" down\n"); else print(" up\n");
	if (layout[row][col] == 0) return;
	if (newstate) keyboard_bitmap_set(layout[row][col]);
	else keyboard_bitmap_clr(layout[row][col]);
}

int main(void)
{
	// CPU Prescaler
	// Set for 16 MHz clock
	CLKPR = 0x80;
	CLKPR = 0;

	// TWI Prescaler
	TWSR = 0;
  	TWBR = 10;

  	// Disable ADC for a tiny power saving
  	ADCSRA = 0;

  	// Millisecond timer requires a prescaling of 64
  	TCCR0A = 0;
	TCCR0B = 3;
	TIMSK0 = (1<<TOIE0);
	sei();

	// Configure pins for righthand:
	//          DDR=1   DDR=0
	// PORT=1   high    pull-up
	// PORT=0   low     floating
	// Input pins are configured as pull-up.
	// Output pins are configured as floating and are toggled to low via DDR to select columns.
	// Unused pins are configured as pull-up.
	// LED pins are configured as high.
    DDRB = 0b01000000; PORTB = 0b11110000; //B0-3         are columns 7-10  (output)  B5,6,7 are LEDa,b,c.
    DDRC = 0b00000000; PORTC = 0b10111111; //C6           is  column  13    (output)
    DDRD = 0b11100000; PORTD = 0b11110011; //D2,3         are columns 11,12 (output)  D6 is Teensy LED.
    DDRE = 0b00000000; PORTE = 0b11111111; //                               (unused)
    DDRF = 0b00000000; PORTF = 0b11111111; //F0,1,4,5,6,7 are R. rows 0-5   (input)
    // Note that at these points all LEDs are ON.

    // Initialize USB.
    // Note if this will wait forever if not connected to PC thus all LED solid on == failure to init.
	usb_init();
	while (!usb_configured()) { idle_ms(1); }
	
	idle_ms(800);
	print("Ready.\n");
	// Turn off all LEDs
	led_set(0);

	// This is located here as a delay is required between TWI init and use.
	// Kick off left hand matrix scanning.
	mcp23018_begin();

	// Is LH connected?
	uint8_t LHconnected = 0;

	// Statistics
	uint16_t counter = 0;
	uint8_t ms = milliseconds, diffms; uint16_t sleeptime = 0, waketime = 0;
	#define RECORD_TIME(VAR) diffms = milliseconds - ms; VAR += diffms; ms += diffms;

	while (1) {
		// Wait for left hand matrix scan
		if (mcp23018_poll()) {
			LHconnected = 1;
		} else {
			LHconnected = 0;
			idle_ms(1);
		}

		RECORD_TIME(sleeptime);

		// Scan right hand matrix
		RIGHT_SCANLINE(7,  B,0);
		RIGHT_SCANLINE(8,  B,1);
		RIGHT_SCANLINE(9,  B,2);
		RIGHT_SCANLINE(10, B,3);
		RIGHT_SCANLINE(11, D,2);
		RIGHT_SCANLINE(12, D,3);
		RIGHT_SCANLINE(13, C,6);

		// Debounce
		// TODO? Doesn't really seem necessary at a ~1.5ms sampling time.

		// Detect edges
		uint8_t anydelta = 0;
		do14({
			uint8_t delta = (state[i] ^ matrixscan[i]);
			if (delta) {
				if (delta & 0b00100000) change(0, i, matrixscan[i] & 0b00100000);
				if (delta & 0b00010000) change(1, i, matrixscan[i] & 0b00010000);
				if (delta & 0b00001000) change(2, i, matrixscan[i] & 0b00001000);
				if (delta & 0b00000100) change(3, i, matrixscan[i] & 0b00000100);
				if (delta & 0b00000010) change(4, i, matrixscan[i] & 0b00000010);
				if (delta & 0b00000001) change(5, i, matrixscan[i] & 0b00000001);
			}
			state[i] = matrixscan[i];
			anydelta |= delta;
		});

		if (anydelta) usb_keyboard_send();

		// Print statistics
		counter++;
		if (waketime+sleeptime >= 10000) {
			uint32_t t = 1000*(uint32_t)(waketime+sleeptime);
			pdec16(counter/10);
			print(" scans/sec. ");
			pdec16(t/counter);
			print("us/scan. Sleep:");
			pdec8(100000*(uint32_t)sleeptime/t);
			if (LHconnected) print("%.\n"); else print("%. LH not connected.\n");
			counter = waketime = sleeptime = 0;
		}

		RECORD_TIME(waketime);
	}
}

//-Os: 624 scans/sec. 1602us/scan. Sleep:79%.
//-O3: 667 scans/sec. 1497us/scan. Sleep:81%.
//-O3: 670 scans/sec. 1492us/scan. Sleep:80%.