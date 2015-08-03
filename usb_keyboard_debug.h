#ifndef usb_serial_h__
#define usb_serial_h__

#include <stdint.h>

void usb_init(void);			// initialize everything
uint8_t usb_configured(void);		// is the USB port configured

static void usb_media_send(uint8_t) __attribute__((unused));
static void usb_media_send(uint8_t o) {}
int8_t usb_keyboard_press(uint8_t key, uint8_t modifier);
int8_t usb_keyboard_send(void);
extern volatile uint8_t keyboard_protocol;
void keyboard_modifiers(uint8_t which);
void keyboard_modifiers_release(uint8_t which);
void keyboard_modifiers_override(uint8_t which, uint8_t state);
void keyboard_modifiers_override_release(void);
void keyboard_bitmap_set(uint8_t key);
void keyboard_bitmap_clr(uint8_t key);
extern volatile uint8_t keyboard_leds;

int8_t usb_debug_putchar(uint8_t c);	// transmit a character
void usb_debug_flush_output(void);	// immediately transmit any buffered output
#define USB_DEBUG_HID

#define MODIFIER_CTRL                   0x01
#define MODIFIER_SHIFT                  0x02
#define MODIFIER_ALT                    0x04
#define MODIFIER_GUI                    0x08
#define MODIFIER_LCTRL                  0x01
#define MODIFIER_LSHIFT                 0x02
#define MODIFIER_LALT                   0x04
#define MODIFIER_LGUI                   0x08
#define MODIFIER_RCTRL                  0x10
#define MODIFIER_RSHIFT                 0x20
#define MODIFIER_RALT                   0x40
#define MODIFIER_RGUI                   0x80

#define KEY_A                              4
#define KEY_B                              5
#define KEY_C                              6
#define KEY_D                              7
#define KEY_E                              8
#define KEY_F                              9
#define KEY_G                             10
#define KEY_H                             11
#define KEY_I                             12
#define KEY_J                             13
#define KEY_K                             14
#define KEY_L                             15
#define KEY_M                             16
#define KEY_N                             17
#define KEY_O                             18
#define KEY_P                             19
#define KEY_Q                             20
#define KEY_R                             21
#define KEY_S                             22
#define KEY_T                             23
#define KEY_U                             24
#define KEY_V                             25
#define KEY_W                             26
#define KEY_X                             27
#define KEY_Y                             28
#define KEY_Z                             29
#define KEY_1                             30
#define KEY_2                             31
#define KEY_3                             32
#define KEY_4                             33
#define KEY_5                             34
#define KEY_6                             35
#define KEY_7                             36
#define KEY_8                             37
#define KEY_9                             38
#define KEY_0                             39
#define KEY_ENTER                         40
#define KEY_ESC                           41
#define KEY_BACKSPACE                     42
#define KEY_TAB                           43
#define KEY_SPACE                         44
#define KEY_MINUS                         45
#define KEY_EQUAL                         46
#define KEY_LBRACE                        47
#define KEY_RBRACE                        48
#define KEY_BACKSLASH                     49
#define KEY_NUMBER                        50
#define KEY_SEMICOLON                     51
#define KEY_QUOTE                         52
#define KEY_TILDE                         53
#define KEY_COMMA                         54
#define KEY_PERIOD                        55
#define KEY_SLASH                         56
#define KEY_CAPSLOCK                      57
#define KEY_F1                            58
#define KEY_F2                            59
#define KEY_F3                            60
#define KEY_F4                            61
#define KEY_F5                            62
#define KEY_F6                            63
#define KEY_F7                            64
#define KEY_F8                            65
#define KEY_F9                            66
#define KEY_F10                           67
#define KEY_F11                           68
#define KEY_F12                           69
#define KEY_PRINTSCREEN                   70
#define KEY_SCROLLLOCK                    71
#define KEY_PAUSE                         72
#define KEY_INSERT                        73
#define KEY_HOME                          74
#define KEY_PAGEUP                        75
#define KEY_DELETE                        76
#define KEY_END                           77
#define KEY_PAGEDOWN                      78
#define KEY_RIGHT                         79
#define KEY_LEFT                          80
#define KEY_DOWN                          81
#define KEY_UP                            82
#define KEY_NUMLOCK                       83
#define KEY_KEYPAD_SLASH                  84
#define KEY_KEYPAD_ASTERIX                85
#define KEY_KEYPAD_MINUS                  86
#define KEY_KEYPAD_PLUS                   87
#define KEY_KEYPAD_ENTER                  88
#define KEY_KEYPAD_1                      89
#define KEY_KEYPAD_2                      90
#define KEY_KEYPAD_3                      91
#define KEY_KEYPAD_4                      92
#define KEY_KEYPAD_5                      93
#define KEY_KEYPAD_6                      94
#define KEY_KEYPAD_7                      95
#define KEY_KEYPAD_8                      96
#define KEY_KEYPAD_9                      97
#define KEY_KEYPAD_0                      98
#define KEY_KEYPAD_PERIOD                 99
#define KEY_BROKENPIPE                   100
#define KEY_APPLICATION                  101
// 102 is not an actual key
#define KEY_KEYPAD_EQUALS                103
#define KEY_F13                          104
#define KEY_F14                          105
#define KEY_F15                          106
#define KEY_F16                          107
#define KEY_F17                          108
#define KEY_F18                          109
#define KEY_F19                          110
#define KEY_F20                          111
#define KEY_F21                          112
#define KEY_F22                          113
#define KEY_F23                          114
#define KEY_F24                          115
#define KEY_MUTE                         127  /* Works under OS X */
#define KEY_VOLUMEINC                    128  /* Works under OS X */
#define KEY_VOLUMEDEC                    129  /* Works under OS X */
#define KEY_JAPANESE_COMMA               133  /* JIS only */
#define KEY_JAPANESE_RO                  135  /* JIS only */
#define KEY_JAPANESE_YEN                 137  /* JIS only */
#define KEY_JAPANESE_KANA                144  /* JIS only */
#define KEY_JAPANESE_EISU                145  /* JIS only */

                                              /* OS X     WINDOWS */
#define KEY_MEDIA_NEXTTRACK              228  /*   Y         Y    */
#define KEY_MEDIA_PREVTRACK              229  /*   Y         Y    */
#define KEY_MEDIA_STOP                   230  /*             Y    */
#define KEY_MEDIA_PLAYPAUSE              231  /*   Y         Y    */
#define KEY_MEDIA_MUTE                   232  /*   Y         Y    */
#define KEY_MEDIA_VOLUMEINC              233  /*   Y         Y    */
#define KEY_MEDIA_VOLUMEDEC              234  /*   Y         Y    */
#define KEY_MEDIA_BASSBOOST              235  /*             Y    */
#define KEY_MEDIA_BASSINC                236  /*             Y    */
#define KEY_MEDIA_BASSDEC                237  /*             Y    */
#define KEY_MEDIA_TREBLEINC              238  /*             Y    */
#define KEY_MEDIA_TREBLEDEC              239  /*             Y    */
#define KEY_MEDIA_BRIGHTNESSINC          240  /*             Y    */
#define KEY_MEDIA_BRIGHTNESSDEC          241  /*             Y    */
#define KEY_MEDIA_LAUNCH_EMAIL           242  /*             Y    */
#define KEY_MEDIA_LAUNCH_CALCULATOR      243  /*             Y    */
#define KEY_MEDIA_LAUNCH_BROWSER         244  /*             Y    */
#define KEY_MEDIA_LAUNCH_CONFIG          245  /*             Y    */
#define KEY_MEDIA_BROWSER_SEARCH         246  /*             Y    */
#define KEY_MEDIA_BROWSER_HOME           247  /*             Y    */
#define KEY_MEDIA_BROWSER_BACK           248  /*             Y    */
#define KEY_MEDIA_BROWSER_FORWARD        249  /*             Y    */
#define KEY_MEDIA_BROWSER_STOP           250  /*             Y    */
#define KEY_MEDIA_BROWSER_REFRESH        251  /*             Y    */
#define KEY_MEDIA_BROWSER_BOOKMARKS      252  /*             Y    */
#define KEY_MEDIA_EJECT                  253  /*   Y              */

// Everything below this point is only intended for usb_serial.c
#ifdef USB_SERIAL_PRIVATE_INCLUDE
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40

#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06

#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

#define MAX_ENDPOINT		4

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#if defined(__AVR_AT90USB162__)
#define HW_CONFIG() 
#define PLL_CONFIG() (PLLCSR = ((1<<PLLE)|(1<<PLLP0)))
#define USB_CONFIG() (USBCON = (1<<USBE))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_ATmega32U4__)
#define HW_CONFIG() (UHWCON = 0x01)
#define PLL_CONFIG() (PLLCSR = 0x12)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_AT90USB646__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x1A)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_AT90USB1286__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x16)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#endif

// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
// CDC (communication class device)
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22
#endif
#endif
