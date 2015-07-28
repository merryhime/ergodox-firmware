# MerryMage's firmware for the Ergodox

Ergodox ([ergodox.org] (http://www.ergodox.org)) is a split fully-reprogrammable ergonomic keyboard.

This project was written for three reasons:

1. The I&sup2;C code is slow in most avaliable firmware.
2. Most firmware doesn't care about power consumption.
3. Learning experience! I haven't written a moderately-sized embedded program for the Teensy before.

This firmware features:

* Faster matrix scans thanks to fast asynchronous I&sup2;C (682 scans/sec, ~1.5 ms/scan)
* NKRO (with boot protocol support)
* Teensy CPU idles whenever possible to save power (about 80% of the time)

To-do:

* Layers, More advanced keyboard layouts
* Media keys
* Mouse keys

## Acknowledgement

This firmware adapts code from PJRC for the USB stack.