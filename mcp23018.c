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
#include <util/twi.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "print.h"

#define TWI_DEBUG(a) /*nothing*/

#define MCP23018_ADDR_WRITE 0b01000000
#define MCP23018_ADDR_READ  0b01000001
#define MCP23018_IODIRA 0x00
#define MCP23018_IODIRB 0x01
#define MCP23018_GPPUA 0x0C
#define MCP23018_GPPUB 0x0D
#define MCP23018_GPIOA 0x12
#define MCP23018_GPIOB 0x13
#define MCP23018_OLATA 0x14
#define MCP23018_OLATB 0x15

#define TWI_SM_CONTINUE 0
#define TWI_SM_OK 1
#define TWI_SM_ERR 2

// twi_sm functions return 1 if OK and 0 on ERROR

#define TWI_SM(NAME, ARGS1, DO1, DO2) \
	static uint8_t twi_sm_##NAME##1 ARGS1 DO1 \
	static uint8_t twi_sm_##NAME##2 () { uint8_t twst = TW_STATUS; DO2 }

TWI_SM(start, (), {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_START);
});

TWI_SM(repstart, (), {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_REP_START);
});

TWI_SM(addr, (uint8_t addr), {
	TWDR = addr;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_MT_SLA_ACK || twst == TW_MR_SLA_ACK);
});

TWI_SM(send, (uint8_t data), {
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_MT_DATA_ACK);
});

// Data is in TWDR register afterwards.
TWI_SM(recvack, (), {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_MR_DATA_ACK);
});

// Data is in TWDR register afterwards.
TWI_SM(recvnak, (), {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_MR_DATA_NACK);
});

TWI_SM(stopstart, (), {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
	return 1;
}, {
	return (twst == TW_START);
});

#include "time.h"

static uint8_t left_scan[7] = {0, 0, 0, 0, 0, 0, 0};
static volatile uint8_t mcp23018_doneflag = 0;
static volatile uint8_t mcp23018_pollwaitflag = 0;
static volatile uint8_t mcp23018_errorcount = 0;
static volatile uint8_t mcp23018_begincallable = 1;

static void twi_sm_syncstop() {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
	while (TWCR & (1<<TWSTO)) {}
	mcp23018_begincallable = 1;
}

void mcp23018_begin() {
	if (!mcp23018_begincallable) { print("mcp23018_begin called in invalid state\n"); usb_debug_flush_output(); }
	mcp23018_begincallable = 0;
	twi_sm_start1();
}

extern uint8_t matrixscan[14];
uint8_t mcp23018_poll() {
	if (mcp23018_errorcount >= 2) {
		// Could not detect left hand
		static uint8_t restartcounter = 1;
		restartcounter++;
		if (restartcounter == 0 && mcp23018_begincallable) mcp23018_begin(); // Attempt to detect LH once every 256 scans (once every 0.4s).
		return 0;
	}

	mcp23018_pollwaitflag = 1;
	while (!mcp23018_doneflag && mcp23018_errorcount < 2) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
	mcp23018_doneflag = 0;
	mcp23018_pollwaitflag = 0;
	matrixscan[0] = left_scan[0];
	matrixscan[1] = left_scan[1];
	matrixscan[2] = left_scan[2];
	matrixscan[3] = left_scan[3];
	matrixscan[4] = left_scan[4];
	matrixscan[5] = left_scan[5];
	matrixscan[6] = left_scan[6];
	if (mcp23018_begincallable && mcp23018_errorcount < 2) {
		mcp23018_begin();
	}
	return 1;
}

// TWIE bit of TWCR needs to be set for this to be called when TWINT is raised.
#define DO(CALL) if (CALL) state++; else goto err;
ISR(TWI_vect) {
	static uint8_t state = 0;
	static uint8_t col = 0;
	switch (state) {
	// MCP23018 Init:
	restart: state = 0;
	case  0: DO(twi_sm_start2());
	case  1: DO(twi_sm_addr1(MCP23018_ADDR_WRITE)); return;
	case  2: DO(twi_sm_addr2());
	case  3: DO(twi_sm_send1(MCP23018_IODIRA)); return;
	case  4: DO(twi_sm_send2());
	case  5: DO(twi_sm_send1(0b00000000)); return;
	case  6: DO(twi_sm_send2());
	case  7: DO(twi_sm_send1(0b00111111)); return;
	case  8: DO(twi_sm_send2());
	case  9: DO(twi_sm_stopstart1()); return;
	case 10: DO(twi_sm_stopstart2());
	case 11: DO(twi_sm_addr1(MCP23018_ADDR_WRITE)); return;
	case 12: DO(twi_sm_addr2());
	case 13: DO(twi_sm_send1(MCP23018_GPPUA)); return;
	case 14: DO(twi_sm_send2());
	case 15: DO(twi_sm_send1(0b00000000)); return;
	case 16: DO(twi_sm_send2());
	case 17: DO(twi_sm_send1(0b00111111)); return;
	case 18: DO(twi_sm_send2());
	case 19: DO(twi_sm_stopstart1()); return;
	// Read
	read:
	case 20: DO(twi_sm_stopstart2());
	case 21: DO(twi_sm_addr1(MCP23018_ADDR_WRITE)); return;
	case 22: DO(twi_sm_addr2());
	case 23: DO(twi_sm_send1(MCP23018_GPIOA)); return;
	case 24: DO(twi_sm_send2());
	case 25: DO(twi_sm_send1(~(1 << col))); return;
	case 26: DO(twi_sm_send2());
	case 27: DO(twi_sm_stopstart1()); return;
	case 28: DO(twi_sm_stopstart2());
	case 29: DO(twi_sm_addr1(MCP23018_ADDR_WRITE)); return;
	case 30: DO(twi_sm_addr2());
	case 31: DO(twi_sm_send1(MCP23018_GPIOB)); return;
	case 32: DO(twi_sm_send2());
	case 33: DO(twi_sm_repstart1()); return;
	case 34: DO(twi_sm_repstart2());
	case 35: DO(twi_sm_addr1(MCP23018_ADDR_READ)); return;
	case 36: DO(twi_sm_addr2());
	case 37: DO(twi_sm_recvnak1()); return;
	case 38: DO(twi_sm_recvnak2());
	case 39: {
		uint8_t recv = TWDR;

		left_scan[col] = (~recv) & 0b00111111;
		
		col++;
		if (col <= 6) {
			state++;
		} else {
			mcp23018_errorcount = 0;
			
			col = 0;
			mcp23018_doneflag = 1;

			if (mcp23018_pollwaitflag) {
				state++;
			} else {
				print("Time Budget Exceeded.\n");
				state = 20;
				twi_sm_syncstop();
				return;
			}
		}
	}
	case 40: DO(twi_sm_stopstart1()); state = 20; return;
	default:
		print("Why are you even here?\n");
	}
err:
	twi_sm_syncstop();
	if (mcp23018_errorcount < 2) {
		pdec8(state); print(" desynced.\n");
		mcp23018_errorcount++;
		state = 0;
		twi_sm_start1();
		mcp23018_begincallable = 0;
		return;
	} else {
		if (mcp23018_errorcount == 2) print("LH lost\n");
		state = 0;
		mcp23018_begincallable = 1;
		mcp23018_errorcount = 3; // prevent overflow
	}
}