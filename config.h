// If 1, prints statisticss (scanrate, etc) to debug channel.
#define STATISTICS 0

// Type of USB report. Select one only.
#define USB_REPORT_BITMAP16 1 // Provides NKRO to all keys listed in usb_keyboard_debug.h.
#define USB_REPORT_BITMAP32 0 // Provides NKRO to all keys including obsolete ones.
#define USB_REPORT_BOOT     0 // Provides 6KRO. Standard USB keyboard mode.

// Mode 0 : Boot Protocol
// Mode 1 : Report Protocol
// Your operating system will set the protocol mode to 1 when the keyboard is plugged in.
// During boot, the older BIOSes require the keyboard to be in boot mode, hence why this is 0.
// If you do not desire this behaviour change this value.
// (Note: The Teensy LED flashes while the keyboard remains in mode 0, informing you the keyboard only has 6KRO.)
#define DEFAULT_PROTOCOL_MODE 0

// If 1, resets the left hand every 4 seconds
// I have no idea why this would be useful to anyone but I've implemented it and that's why it's here.
#define MCP23018_FORCERESET 0

// If 1, we use a Apple JIS keyboard VID/PID
#define USB_OSX_JIS_HACK 0

// Hang the keyboard, internal error
void hang(const char *s);