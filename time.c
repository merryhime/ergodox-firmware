#include "config.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "print.h"
#include "time.h"

volatile uint32_t milliseconds=0;
static uint8_t msfraction;

void time_init(void) {
	// Prescaler of 64
	TCCR0A = 0;
	TCCR0B = 3;
	TIMSK0 = (1<<TOIE0);
}

void idle_ms(uint16_t ms) {
	uint32_t target = milliseconds+ms;
	while (target > milliseconds) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
}

// Ticks once every 1.024 milliseconds = 256*64/(16MHz)
// Error of +0.024ms per call = 24/1000 ms = 3/125 ms
ISR(TIMER0_OVF_vect)
{
	msfraction += 3;
	if (msfraction >= 125) {
		msfraction -= 125;
		milliseconds += 2;
	} else {
		milliseconds++;
	}
	sleep_disable();
}