# Name: Makefile
# Project: MidiFoot
# Author: Bill Peterson
# Creation Date: 2020-12-01
# Tabsize: 4
# License: MIT

DEVICE=attiny85
AVRDUDE = avrdude -c linuxspi -P /dev/spidev0.0 -p $(DEVICE) -b 9600
# The two lines above are for "avrdude" and the SPI pins on a Raspberry Pi
# Choose your favorite programmer and interface.

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o midifoot.o

# symbolic targets:
all:	midifoot.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:midifoot.hex:i

fuse:
	$(AVRDUDE) -U hfuse:w:0xdf:m -U lfuse:w:0xef:m

readcal:
	$(AVRDUDE) -U calibration:r:/dev/stdout:i | head -1

clean:
	rm -f midifoot.hex midifoot.lst midifoot.obj midifoot.cof midifoot.list midifoot.map midifoot.eep.hex midifoot.bin *.o usbdrv/*.o midifoot.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
midifoot.bin:	$(OBJECTS)
	$(COMPILE) -o midifoot.bin $(OBJECTS)

midifoot.hex:	midifoot.bin
	rm -f midifoot.hex midifoot.eep.hex
	avr-objcopy -j .text -j .data -O ihex midifoot.bin midifoot.hex
	./checksize midifoot.bin 4096 256
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	midifoot.bin
	avr-objdump -d midifoot.bin

cpp:
	$(COMPILE) -E midifoot.c
