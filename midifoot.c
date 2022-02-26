/* Name: midifoot.c
 * Project: Single button midi controller
 * Author: Bill Peterson
 */

#include <avr/io.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/wdt.h>

#include "usbdrv.h"
#if USE_INCLUDE
#include "usbdrv.c"
#endif

// This descriptor is based on http://www.usb.org/developers/devclass_docs/midi10.pdf
// 
// Appendix B. Example: Simple MIDI Adapter (Informative)
// B.1 Device Descriptor
const static PROGMEM char deviceDescrMIDI[] = {    /* USB device descriptor */
    18,            /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,    /* descriptor type */
    0x10, 0x01,        /* USB version supported */
    0,            /* device class: defined at interface level */
    0,            /* subclass */
    0,            /* protocol */
    8,            /* max packet size */
    USB_CFG_VENDOR_ID,    /* 2 bytes */
    USB_CFG_DEVICE_ID,    /* 2 bytes */
    USB_CFG_DEVICE_VERSION,    /* 2 bytes */
    1,            /* manufacturer string index */
    2,            /* product string index */
    0,            /* serial number string index */
    1,            /* number of configurations */
};

// B.2 Configuration Descriptor
const static PROGMEM char configDescrMIDI[] = {    /* USB configuration descriptor */
    9,            /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    101, 0,            /* total length of data returned (including inlined descriptors) */
    2,            /* number of interfaces in this configuration */
    1,            /* index of this configuration */
    0,            /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    USBATTR_SELFPOWER,    /* attributes */
#else
    USBATTR_BUSPOWER,    /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER / 2,    /* max USB current in 2mA units */

// B.3 AudioControl Interface Descriptors
// The AudioControl interface describes the device structure (audio function topology) 
// and is used to manipulate the Audio Controls. This device has no audio function 
// incorporated. However, the AudioControl interface is mandatory and therefore both 
// the standard AC interface descriptor and the classspecific AC interface descriptor 
// must be present. The class-specific AC interface descriptor only contains the header 
// descriptor.

// B.3.1 Standard AC Interface Descriptor
// The AudioControl interface has no dedicated endpoints associated with it. It uses the 
// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl 
// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
    /* AC interface descriptor follows inline: */
    9,            /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE,    /* descriptor type */
    0,            /* index of this interface */
    0,            /* alternate setting for this interface */
    0,            /* endpoints excl 0: number of endpoint descriptors to follow */
    1,            /* */
    1,            /* */
    0,            /* */
    0,            /* string index for interface */

// B.3.2 Class-specific AC Interface Descriptor
// The Class-specific AC interface descriptor is always headed by a Header descriptor 
// that contains general information about the AudioControl interface. It contains all 
// the pointers needed to describe the Audio Interface Collection, associated with the 
// described audio function. Only the Header descriptor is present in this device 
// because it does not contain any audio functionality as such.
    /* AC Class-Specific descriptor */
    9,            /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    36,            /* descriptor type */
    1,            /* header functional descriptor */
    0x0, 0x01,        /* bcdADC */
    9, 0,            /* wTotalLength */
    1,            /* */
    1,            /* */

// B.4 MIDIStreaming Interface Descriptors

// B.4.1 Standard MS Interface Descriptor
    /* interface descriptor follows inline: */
    9,            /* length of descriptor in bytes */
    USBDESCR_INTERFACE,    /* descriptor type */
    1,            /* index of this interface */
    0,            /* alternate setting for this interface */
    2,            /* endpoints excl 0: number of endpoint descriptors to follow */
    1,            /* AUDIO */
    3,            /* MS */
    0,            /* unused */
    0,            /* string index for interface */

// B.4.2 Class-specific MS Interface Descriptor
    /* MS Class-Specific descriptor */
    7,            /* length of descriptor in bytes */
    36,            /* descriptor type */
    1,            /* header functional descriptor */
    0x0, 0x01,        /* bcdADC */
    65, 0,            /* wTotalLength */

// B.4.3 MIDI IN Jack Descriptor
    6,            /* bLength */
    36,            /* descriptor type */
    2,            /* MIDI_IN_JACK desc subtype */
    1,            /* EMBEDDED bJackType */
    1,            /* bJackID */
    0,            /* iJack */

    6,            /* bLength */
    36,            /* descriptor type */
    2,            /* MIDI_IN_JACK desc subtype */
    2,            /* EXTERNAL bJackType */
    2,            /* bJackID */
    0,            /* iJack */

//B.4.4 MIDI OUT Jack Descriptor
    9,            /* length of descriptor in bytes */
    36,            /* descriptor type */
    3,            /* MIDI_OUT_JACK descriptor */
    1,            /* EMBEDDED bJackType */
    3,            /* bJackID */
    1,            /* No of input pins */
    2,            /* BaSourceID */
    1,            /* BaSourcePin */
    0,            /* iJack */

    9,            /* bLength of descriptor in bytes */
    36,            /* bDescriptorType */
    3,            /* MIDI_OUT_JACK bDescriptorSubtype */
    2,            /* EXTERNAL bJackType */
    4,            /* bJackID */
    1,            /* bNrInputPins */
    1,            /* baSourceID (0) */
    1,            /* baSourcePin (0) */
    0,            /* iJack */


// B.5 Bulk OUT Endpoint Descriptors

//B.5.1 Standard Bulk OUT Endpoint Descriptor
    9,            /* bLenght */
    USBDESCR_ENDPOINT,    /* bDescriptorType = endpoint */
    0x1,            /* bEndpointAddress OUT endpoint number 1 */
    3,            /* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
    8, 0,            /* wMaxPacketSize */
    10,            /* bIntervall in ms */
    0,            /* bRefresh */
    0,            /* bSyncAddress */

// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
    5,            /* bLength of descriptor in bytes */
    37,            /* bDescriptorType */
    1,            /* bDescriptorSubtype */
    1,            /* bNumEmbMIDIJack  */
    1,            /* baAssocJackID (0) */


//B.6 Bulk IN Endpoint Descriptors

//B.6.1 Standard Bulk IN Endpoint Descriptor
    9,            /* bLenght */
    USBDESCR_ENDPOINT,    /* bDescriptorType = endpoint */
    0x81,            /* bEndpointAddress IN endpoint number 1 */
    3,            /* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
    8, 0,            /* wMaxPacketSize */
    10,            /* bIntervall in ms */
    0,            /* bRefresh */
    0,            /* bSyncAddress */

// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
    5,            /* bLength of descriptor in bytes */
    37,            /* bDescriptorType */
    1,            /* bDescriptorSubtype */
    1,            /* bNumEmbMIDIJack (0) */
    3,            /* baAssocJackID (0) */
};

// provide the custom descriptor
usbMsgLen_t usbFunctionDescriptor(usbRequest_t * rq)
{
    if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
        usbMsgPtr = (uchar *) deviceDescrMIDI;
        return sizeof(deviceDescrMIDI);
    } else {        /* must be config descriptor */
        usbMsgPtr = (uchar *) configDescrMIDI;
        return sizeof(configDescrMIDI);
    }
}

// no special setup needed
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    return 0;
}

// midi packets
// byte 0: packet header - cable number (always 0), code index (msg type)
// midi byte 1 - msg type, channel
// midi byte 2 - note or CC number
// midi byte 3 - velocity or value
//  msg types:
//   8 - note-on
//   9 - note-off
//   A - poly-keypress
//   B - control change
//   C - program change
//   D - channel pressure
//   E - pitch bend
uchar midiPkt[16][4] = {
    {0x0B, 0xBE, 0x40, 0x46}, // [0]  ch15 cc64 (hold pedal) =  70 (on)
    {0x0B, 0xBE, 0x40, 0x00}, // [1]  ch15 cc64 (hold pedal) =   0 (off)
    {0x0B, 0xBE, 0x40, 0x5a}, // [2]  ch15 cc64 (hold pedal) =  90 (on)
    {0x0B, 0xBE, 0x40, 0x14}, // [3]  ch15 cc64 (hold pedal) =  20 (off)
    {0x0B, 0xBE, 0x40, 0x50}, // [4]  ch15 cc64 (hold pedal) =  80 (on)
    {0x0B, 0xBE, 0x40, 0x0a}, // [5]  ch15 cc64 (hold pedal) =  10 (off)
    {0x0B, 0xBE, 0x40, 0x64}, // [6]  ch15 cc64 (hold pedal) = 100 (on)
    {0x0B, 0xBE, 0x40, 0x1e}, // [7]  ch15 cc64 (hold pedal) =  30 (off)
    {0x0B, 0xBE, 0x40, 0x47}, // [8]  ch15 cc64 (hold pedal) =  71 (on)
    {0x0B, 0xBE, 0x40, 0x01}, // [9]  ch15 cc64 (hold pedal) =   1 (off)
    {0x0B, 0xBE, 0x40, 0x5b}, // [10] ch15 cc64 (hold pedal) =  91 (on)
    {0x0B, 0xBE, 0x40, 0x15}, // [11] ch15 cc64 (hold pedal) =  21 (off)
    {0x0B, 0xBE, 0x40, 0x51}, // [12] ch15 cc64 (hold pedal) =  81 (on)
    {0x0B, 0xBE, 0x40, 0x0b}, // [13] ch15 cc64 (hold pedal) =  11 (off)
    {0x0B, 0xBE, 0x40, 0x65}, // [14] ch15 cc64 (hold pedal) = 101 (on)
    {0x0B, 0xBE, 0x40, 0x1f}, // [15] ch15 cc64 (hold pedal) =  31 (off)
};
#define MSG_COUNT 16
uint8_t msgNum = 0;
uint8_t lastReading = 1;
uint8_t buttonState = 1;

static inline void initTimer1(void)
{
    TCCR1 |= (1 << CTC1);       // clear timer on compare match
    TCCR1 |= (1 << CS13);       // clock prescaler 128
    OCR1C = 25;                 // compare match value to trigger interrupt every 200us ([1 / (16E6 / 128)] * 25 = 200us)
    TIMSK |= (1 << OCIE1A);     // enable output compare match interrupt
    sei();                      // enable interrupts
}

ISR(TIMER1_COMPA_vect)
{
    buttonState = PINB & (1 << PB0);
    if (buttonState != lastReading)
    {
        if (usbInterruptIsReady())
        {
            usbSetInterrupt(midiPkt[msgNum], 4);
            msgNum++;
			if (msgNum >= MSG_COUNT) msgNum = 0;
        }
    }
    lastReading = buttonState;
}

int main(void)
{
    MCUSR = 0;
    wdt_disable();
    usbInit();
    usbDeviceDisconnect(); // enforce re-enumeration
    uint8_t i = 0;
    while(--i) // fake USB disconnect for > 250 ms
    {
        _delay_ms(1);
    }
    usbDeviceConnect();
    wdt_enable(WDTO_500MS);
    sei();

    DDRB &= ~(1 << PB0);        // set PB0 as input (default)
    PORTB |= (1 << PB0);        // enable pullup on PB0

    initTimer1();               // initialize timer and interrupt
    
    for(;;) // main event loop
    {
        wdt_reset(); // reset the watchdog timer
        usbPoll();
    }
    return 0;
}
