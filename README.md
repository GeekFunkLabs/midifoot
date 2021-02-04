# MidiFoot
The MidiFoot is a USB MIDI controller with a single button. It sends different batches of messages when you press and release the button (shown below), which you can map to whatever you need to control in your sound module/DAW. It uses an ATTiny85 to emulate a class-compliant USB MIDI device using the [V-USB](https://www.obdev.at/products/vusb/index.html) library, so it’s inexpensive and doesn’t need any drivers installed to work on most computers. The PID=1508 and VID=5824 have been generously sponsored by [Objective Development](https://www.obdev.at/). The MidiFoot is a creation of [Geek Funk Labs](http://geekfunklabs.com), where you can find a complete bill of materials, links to obtain kits/builds, etc.

MidiFoot sends the following MIDI messages on the specified MIDI channels each time you press and release the button:

- Sustain pedal (CC 64) on channel 1
- Kick drum (note 36) on channel 10
- CC 16 on channel 15 momentary (press=127, release=0)
- CC 17 on channel 15 toggle (first press=127, second press=0)
- A repeating four-note pattern:
  - first press: note 36 (kick drum) on channel 15
  - first release: note 46 (open hihat) on channel 15
  - second press: note 38 (snare) on channel 15
  - second release: note 33 (ride) on channel 15
  
## Installing/Flashing

You will need an AVR-C environment installed to build from source. A Raspberry Pi is quite useful as a build environment, as you can flash AVRs using the SPI interface on the GPIO header. To install AVR-GCC on the Pi, enter
```
sudo apt install gcc-avr
```
Make the following connections to program the ATTiny85:
RPi Physical Pin | ATTiny85
-------------|-----------
23 (SCLK) | 7 (SCK)
21 (MISO) | 6 (MISO)
19 (MOSI) | 5 (MOSI)
22 (GPIO25) | 1 (RESET)
2 (5V) | 8 (VCC)
6 (GND) | 4 (GND)

You must also connect a 16MHz clock source (quartz oscillator) to pins 2/3 (XTAL1/2) on the ATTiny, and ground each pin through a 22pF capacitor. To compile the software, flash the chip, and set the fuses, enter
```
sudo make flash && sudo make fuse
```
