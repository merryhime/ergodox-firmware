
// USB Documentation
// See Atmel datasheet chapters 21 and 22. Very sparse on information.
// Also see USB HID 1.11: http://www.usb.org/developers/hidpage/HID1_11.pdf

// HID Section 6.2.1 HID Descriptor
typedef __attribute((packed))__ struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorType;
	uint8_t wDescriptorLength;
	uint8_t bDescr
} HidDescriptor_t;

void usb_init() {
	UHWCON = 0b00000001; //set UVREGE, enabling USB pad regulator
	USBCON = 0b10100000; //USBE (enable USB controller), FRZCLK (freeze clock - powersave)

}