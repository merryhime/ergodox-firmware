#include "config.h"
#include "translator.h"
#include "usb_keyboard_debug.h"
#include "time.h"
#include "print.h"

typedef void (*layout_fn)(uint8_t* data, uint8_t row, uint8_t col, uint8_t down);
static void layout_null(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_fallthrough(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_key(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_keyshift(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_media(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_mod(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_layer(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_stickymod(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_stickylayer(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_mod_tap(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_layer_tap(uint8_t*, uint8_t, uint8_t, uint8_t) __attribute__((unused));
static void layout_keyreleased(void);

const static layout_fn LAYOUT_FN[] = {
	&layout_null,
	&layout_fallthrough,
	&layout_key,
	&layout_key,
	&layout_keyshift,
	&layout_media,
	&layout_mod,
	&layout_layer,
	&layout_stickymod,
	&layout_stickylayer,
	&layout_mod_tap,
	&layout_layer_tap
};

#define VOID            (uint8_t *)(0)
#define NULL            (uint8_t[]){0}
#define FALL            (uint8_t[]){1}
#define k(k)            (uint8_t[]){2,  KEY_##k}
#define kp(k)           (uint8_t[]){3,  KEY_KEYPAD_##k}
#define K(k)            (uint8_t[]){4,  KEY_##k}
#define km(k)           (uint8_t[]){5,  KEY_MEDIA_##k}
#define m(m)            (uint8_t[]){6,  MODIFIER_##m}
#define L(l)            (uint8_t[]){7,  l}
#define sm(mod)         (uint8_t[]){8,  MODIFIER_##mod, 0}
#define slayer(l)       (uint8_t[]){9,  l,              0}
#define mtap(mod, k)    (uint8_t[]){10, MODIFIER_##mod, KEY_##k, 0}
#define ltap(l, k)      (uint8_t[]){11, l,              KEY_##k, 0}

/* static uint8_t* layout[6][14] = {
	{k(ESC),   k(1), k(2), k(3), k(4), k(5), k(MINUS),            kp(PLUS),  k(6), k(7), k(8), k(9), k(0), k(ENTER)}, 
	{K(MINUS), k(Q), k(W), k(F), k(P), k(G), K(9),                K(0),      k(J), k(L), k(U), k(J), k(SEMICOLON), k(EQUAL)}, 
	{m(LCTRL), k(A), k(R), k(S), k(T), k(D), VOID,                VOID,      k(H), k(N), k(E), k(I), k(O), k(QUOTE)},
	{k(TILDE), k(Z), k(X), k(C), k(V), k(B), k(LBRACE),           k(RBRACE), k(K), k(M), k(COMMA), k(PERIOD), k(SLASH), k(BACKSLASH)},
	{NULL,     NULL, NULL, NULL, m(LGUI), VOID, VOID,             VOID,      VOID, m(RGUI), NULL, NULL, NULL, NULL},
	{VOID,     NULL, sm(LSHIFT), k(SPACE), sm(LALT), NULL, NULL,  NULL,      NULL, sm(RALT), k(BACKSPACE), k(ENTER), NULL, VOID}
}; */

static uint8_t* layout[6][14] = {
	{k(ESC),   k(1), k(2), k(3), k(4), k(5), k(MINUS),            kp(PLUS),  k(6), k(7), k(8), k(9), k(0), k(ENTER)}, 	s
	{K(MINUS), k(Q), k(W), k(E), k(R), k(T), K(9),                K(0),      k(Y), k(U), k(I), k(O), k(P), k(EQUAL)}, 
	{m(LCTRL), k(A), k(S), k(D), k(F), k(G), VOID,                VOID,      k(H), k(J), k(K), k(L), k(SEMICOLON), k(QUOTE)},
	{k(TILDE), k(Z), k(X), k(C), k(V), k(B), k(LBRACE),           k(RBRACE), k(N), k(M), k(COMMA), k(PERIOD), k(SLASH), k(BACKSLASH)},
	{NULL,     NULL, NULL, NULL, m(LGUI), VOID, VOID,             VOID,      VOID, m(RGUI), NULL, NULL, NULL, NULL},
	{VOID,     NULL, sm(LSHIFT), k(SPACE), sm(LALT), NULL, NULL,  NULL,      NULL, sm(RALT), k(BACKSPACE), k(ENTER), NULL, VOID}
};

void press(uint8_t k) {
	keyboard_bitmap_set(k);
	usb_keyboard_send();
	idle_ms(1);
	keyboard_bitmap_clr(k);
	usb_keyboard_send();
	idle_ms(1);
}

static uint8_t* upfn[6][14] = {
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0}
};

// row:col is as labelled on the PCB (SWrow:col)
// down == 1 if key down, == 0 if key up
static void translate(uint8_t row, uint8_t col, uint8_t down) {
	row = 5 - row;
	if (layout[row][col] == 0) hang("translate: VOID\n");

	if (down) {
		uint8_t* data = layout[row][col];
		upfn[row][col] = data;

		LAYOUT_FN[data[0]](data, row, col, down);
	} else {
		if (!upfn[row][col]) hang("translate: upfn desync\n");
		uint8_t* data = upfn[row][col];
		upfn[row][col] = 0;

		LAYOUT_FN[data[0]](data, row, col, down);
	}
}

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

// Manually unrolled 0..6 loop.
#define do6(OP) \
	do { const uint8_t j = 0, mask = 0b00100000; OP; } while(0); \
	do { const uint8_t j = 1, mask = 0b00010000; OP; } while(0); \
	do { const uint8_t j = 2, mask = 0b00001000; OP; } while(0); \
	do { const uint8_t j = 3, mask = 0b00000100; OP; } while(0); \
	do { const uint8_t j = 4, mask = 0b00000010; OP; } while(0); \
	do { const uint8_t j = 5, mask = 0b00000001; OP; } while(0); \

static uint8_t state[14] = {0, 0, 0, 0, 0, 
                            0, 0, 0, 0, 0, 
                            0, 0, 0, 0};

void translate_tick(uint8_t matrixscan[14]) {
	uint8_t delta, down;
	do14({
		delta = (state[i] ^ matrixscan[i]);
		if (delta) {
			do6({
				if (delta & mask) {
					down = !!(matrixscan[i] & mask);
					translate(j, i, down);
					if (down) state[i] |= mask;
					else state[i] &= ~mask;
					delta = (state[i] ^ matrixscan[i]);
				}
			});
		}
	});
}

static void layout_null(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	return; // Do nothing
}

static void layout_fallthrough(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	//TODO
}

static void layout_key(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t k = data[1];
	if (down) {
		keyboard_bitmap_set(k); usb_keyboard_send();
	} else {
		keyboard_bitmap_clr(k); usb_keyboard_send();
		layout_keyreleased();
	}
}

static void layout_keyshift(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t k = data[1];
	if (down) {
		keyboard_modifiers_override(MODIFIER_LSHIFT | MODIFIER_RSHIFT, MODIFIER_LSHIFT); usb_keyboard_send();
		idle_ms(1);
		keyboard_bitmap_set(k); usb_keyboard_send();
	} else {
		keyboard_bitmap_clr(k); usb_keyboard_send();
		idle_ms(1);
		keyboard_modifiers_override_release(); usb_keyboard_send();
		layout_keyreleased();
	}
}

static void layout_media(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t k = data[1];
	if (down) {
		usb_media_send(k);
	} else {
		usb_media_send(0);
		layout_keyreleased();
	}
}

static void layout_mod(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t m = data[1];
	if (down) {
		keyboard_modifiers(m); usb_keyboard_send();
	} else {
		keyboard_modifiers_release(m); usb_keyboard_send();
	}
}

static void layout_layer(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t l = data[1];
	(void)l;
}

// Draw out the state machine for this to make sense.
// Sometimes I wish I could embed graphics in comments.
static void layout_stickymod(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t m = data[1];
	switch (data[2]) {
	default:
		if (down == 1) {
			keyboard_modifiers(m); usb_keyboard_send();
			data[2] = 1;
		}
		return;
	case 1:
		if (down == 0) {
			upfn[row][col] = data;
			data[2] = 2;
		} else if (down == 2) {
			data[2] = 3;
		}
		return;
	case 2:
		if (down == 1) {
			data[2] = 3;
		} else if (down == 2) {
			keyboard_modifiers_release(m); usb_keyboard_send();
			data[2] = 0;
		}
		return;
	case 3:
		if (down == 0) {
			keyboard_modifiers_release(m); usb_keyboard_send();
			data[2] = 0;
		}
		return;
	}
}

static void layout_stickylayer(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t l = data[1];
	(void) l;
}

static void layout_mod_tap(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t m = data[1], k = data[2]; 
	if (down) {
		keyboard_modifiers(m); usb_keyboard_send();
		data[3] = 0;
		return;
	} else {
		keyboard_modifiers_release(m); usb_keyboard_send();
		if (!data[3]) {
			idle_ms(1);
			keyboard_bitmap_set(k); usb_keyboard_send();
			idle_ms(1);
			keyboard_bitmap_clr(k); usb_keyboard_send();
			layout_keyreleased();
		}
		data[3] = 0;
		return;
	}
}

static void layout_layer_tap(uint8_t* data, uint8_t row, uint8_t col, uint8_t down) {
	uint8_t l = data[1], k = data[2]; data[3] = 0;
	(void) l;
	(void) k;
}

static void layout_keyreleased() {
	uint8_t* data;
	do14({
		do6({
			data = upfn[j][i];
			if (data) {
				if (data[0] == 8 || data[0] == 9) {
					LAYOUT_FN[data[0]](data, j, i, 2);
				} else if (data[0] == 10 || data[0] == 11) {
					data[3] = 1;
				}
			}
			(void)mask;
		});
	});
}