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

static uint8_t twi_substate = 0;

static uint8_t twi_sm_start() {
	TWI_DEBUG({print("twi_sm_start "); phex(twi_substate); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			//Send start
			TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_START) {
				({print("!TW_START in twi_sm_start\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_repstart() {
	TWI_DEBUG({print("twi_sm_repstart "); phex(twi_substate); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			//Send start
			TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_REP_START) {
				({print("!TW_REP_START in twi_sm_repstart\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_addr(uint8_t addr) {
	TWI_DEBUG({print("twi_sm_addr "); phex(twi_substate); print(" "); phex(addr); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			TWDR = addr;
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_MT_SLA_ACK && twst != TW_MR_SLA_ACK) {
				({print("!TW_Mx_SLA_ACK in twi_sm_addr\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_send(uint8_t data) {
	TWI_DEBUG({print("twi_sm_send "); phex(twi_substate); print(" "); phex(data); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			TWDR = data;
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_MT_DATA_ACK) {
				({print("!TW_MT_DATA_ACK in twi_sm_send\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_recvack(uint8_t* data) {
	TWI_DEBUG({print("twi_sm_recvack "); phex(twi_substate); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_MR_DATA_ACK) {
				({print("!TW_MR_DATA_ACK in twi_sm_recvack\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			*data = TWDR;
			TWI_DEBUG({print("Got "); phex(*data); print("\n");});

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_recvnak(uint8_t* data) {
	TWI_DEBUG({print("twi_sm_recvnak "); phex(twi_substate); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_MR_DATA_NACK) {
				({print("!TW_MR_DATA_NACK in twi_sm_recvnak\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			*data = TWDR;
			TWI_DEBUG({print("Got "); phex(*data); print("\n");});

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static uint8_t twi_sm_stopstart() {
	TWI_DEBUG({print("twi_sm_stopstart "); phex(twi_substate); print("\n");});
	switch (twi_substate) {
	case 0:
		{
			//Send start
			TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWSTO) | (1<<TWEN) | (1<<TWIE);
			twi_substate = 1;
			return TWI_SM_OK;
		}
	default:
		{
			uint8_t twst = TW_STATUS;
			if (twst != TW_START) {
				({print("!TW_START in twi_sm_stopstart\n");});
				twi_substate = 0;
				return TWI_SM_ERR;
			}

			twi_substate = 0;
			return TWI_SM_CONTINUE;
		}
	}
}

static void twi_sm_syncstop() {
	TWI_DEBUG({print("twi_sm_syncstop\n");});
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
	while (TWCR & (1<<TWSTO)) {}
	twi_substate = 0;
}

#define DO(CALL)                              \
	do {                                      \
		switch (CALL) {                       \
		case TWI_SM_OK:                       \
			return;                           \
		case TWI_SM_CONTINUE:                 \
			mcp23018_state++;                 \
			break; /*EXPECTS FALLTHROUGH!!*/  \
		case TWI_SM_ERR:                      \
			goto err;                         \
		}                                     \
	} while(0)

#include "time.h"

static uint8_t left_scan[7] = {0, 0, 0, 0, 0, 0, 0};
static volatile uint8_t mcp23018_doneflag = 0;
static volatile uint8_t mcp23018_pollwaitflag = 0;
static volatile uint8_t mcp23018_errorcount = 0;
static uint8_t mcp23018_state = 0;

void mcp23018_begin() {
	if (mcp23018_state != 0 && mcp23018_state != 22) print("mcp23018_begin called in invalid state\n"); //lolidk
	twi_substate = 0;
	twi_sm_start();
}

extern uint8_t matrixscan[14];
uint8_t mcp23018_poll() {
	if (mcp23018_errorcount >= 2) {
		// Could not detect left hand
		static uint8_t restartcounter = 0;
		restartcounter++;
		if (restartcounter == 0) mcp23018_begin(); // Attempt to detect LH once every 256 scans (once every 0.4s).
		return 0;
	}

	mcp23018_pollwaitflag = 1;
	while (!mcp23018_doneflag && mcp23018_errorcount < 2) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
	mcp23018_doneflag = 0;
	matrixscan[0] = left_scan[0];
	matrixscan[1] = left_scan[1];
	matrixscan[2] = left_scan[2];
	matrixscan[3] = left_scan[3];
	matrixscan[4] = left_scan[4];
	matrixscan[5] = left_scan[5];
	matrixscan[6] = left_scan[6];
	if (mcp23018_errorcount < 2 && mcp23018_pollwaitflag != 2) {
		mcp23018_begin();
	}
	mcp23018_pollwaitflag = 0;
	return 1;
}

// TWIE bit of TWCR needs to be set for this to be called when TWINT is raised.
ISR(TWI_vect) {
	static uint8_t recv = 0;
	static uint8_t col = 0;
	switch (mcp23018_state) {
	// MCP23018 Init:
	restart:
	case 0:  DO(twi_sm_start());
	case 1:  DO(twi_sm_addr(MCP23018_ADDR_WRITE));
	case 2:  DO(twi_sm_send(MCP23018_IODIRA));
	case 3:  DO(twi_sm_send(0b00000000));
	case 4:  DO(twi_sm_send(0b00111111));
	case 5:  DO(twi_sm_stopstart());
	case 6:  DO(twi_sm_addr(MCP23018_ADDR_WRITE));
	case 7:  DO(twi_sm_send(MCP23018_GPPUA));
	case 8:  DO(twi_sm_send(0b00000000));
	case 9:  DO(twi_sm_send(0b00111111));
	case 10: DO(twi_sm_stopstart());
	// Read
	read: mcp23018_state = 11;
	case 11: DO(twi_sm_addr(MCP23018_ADDR_WRITE));
	case 12: DO(twi_sm_send(MCP23018_GPIOA));
	case 13: DO(twi_sm_send(~(1 << col)));
	case 14: DO(twi_sm_stopstart());
	case 15: DO(twi_sm_addr(MCP23018_ADDR_WRITE));
	case 16: DO(twi_sm_send(MCP23018_GPIOB));
	case 17: DO(twi_sm_repstart());
	case 18: DO(twi_sm_addr(MCP23018_ADDR_READ));
	case 19: DO(twi_sm_recvnak(&recv));
	case 20: {
		TWI_DEBUG({phex(col); print(": "); pbin((~recv) & 0b00111111); print(" ");});

		left_scan[col] = (~recv) & 0b00111111;
		
		col++;
		if (col <= 6) {
			mcp23018_state = 21;
			// FALLTHROUGH TO 21
		} else {
			TWI_DEBUG({print("\n");});
			if (mcp23018_errorcount >= 2) print("LH connected\n");
			mcp23018_errorcount = 0;
			
			col = 0;
			mcp23018_doneflag = 1;
			sleep_disable();

			if (mcp23018_pollwaitflag) {
				mcp23018_pollwaitflag = 2;
				mcp23018_state = 21;
			} else {
				print("Time Budget Exceeded.\n");
				mcp23018_state = 22;
				twi_sm_syncstop();
				return;
			}
		}
	}
	case 21: DO(twi_sm_stopstart()); goto read;
	case 22: DO(twi_sm_start()); goto read;
	default:
		TWI_DEBUG({print("Why are you even here?\n");});
	}
err:
	twi_sm_syncstop();
	if (mcp23018_errorcount < 2) {
		pdec8(mcp23018_state); print(" desynced.\n");
		mcp23018_errorcount++;
		mcp23018_state = 0;
		goto restart;
	}
	if (mcp23018_errorcount == 2) print("LH lost\n");
	mcp23018_state = 0;
	mcp23018_errorcount = 3; // prevent overflow
}