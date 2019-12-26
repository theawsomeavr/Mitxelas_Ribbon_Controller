#include "Mitxelas_Ribbon_Controller.h"
/***************************************
 *      Made by Mitxela (Alex Tim)     *
 *   Modefied by David Rubio 25/12/19  *
 ****************************************/
void sendPitchBendRange(){
  uchar pitchBendRange[8] = {0x0B, 0xB0, 101, 0, 0x0B, 0xB0, 100, 0};
  while (!usbInterruptIsReady()){
    usbPoll();
  }
  usbSetInterrupt(pitchBendRange, 8);
  
  while (!usbInterruptIsReady()){
    usbPoll();
  }
  pitchBendRange[2]=6;
  pitchBendRange[3]=8; //Number of +/- semitones
  usbSetInterrupt(pitchBendRange, 4);
}

  uint8_t  i;
  uint8_t  sentPBR =0;
  uint8_t  calibrationValue;
  uint8_t  midiMsg[8];
  uint16_t  val=0;
  uint16_t  lastVal=0;
void setup (){  
  DDRB = 1<<1;
  PORTB |= 1<<4;
  OCR1A = 1;
  TCCR1=0;
  OCR1C=50;
  usbDeviceDisconnect();
  for(i=0;i<20;i++){  /* 300 ms disconnect */
    _delay_ms(15);
  }
  usbDeviceConnect();
  usbInit();
  sei();
}
void loop(){    
    if ((PINB & 1<<PB4)==0 && !sentPBR) {
      sendPitchBendRange();
      sentPBR=1;
    }
    usbPoll();
    if(usbInterruptIsReady()){
      ADMUX = (1<<MUX1 | 1<<MUX0); //PB3
      ADCSRA = (1<<ADEN|1<<ADSC|1<<ADIF|1<<ADPS2|1<<ADPS1);
      while (!(ADCSRA & (1<<ADIF)));
      val = ADCW;
      
      if (abs(val-lastVal)>=1){
        i=0;
        if (lastVal==0) {
          midiMsg[i++]= 0x09;
          midiMsg[i++]= 0x90;
          midiMsg[i++]= 0x40;
          midiMsg[i++]= 0x7F;
        }else if (val==0){
          midiMsg[i++]= 0x08;
          midiMsg[i++]= 0x80;
          midiMsg[i++]= 0x40;
          midiMsg[i++]= 0x00;
        }
        if (val!=0) {
          midiMsg[i++]= 0x0E;
          midiMsg[i++]= 0xE0;
          midiMsg[i++]= val&0x7F;
          midiMsg[i++]= val>>3;
        }
        lastVal=val;
        
        usbSetInterrupt(midiMsg, i);
        sentPBR=0;
      }
    }

  }
  const PROGMEM char deviceDescrMIDI[19] = {  /* USB device descriptor */
    18,     /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,  /* descriptor type */
    0x10, 0x01,   /* USB version supported */
    0,      /* device class: defined at interface level */
    0,      /* subclass */
    0,      /* protocol */
    8,      /* max packet size */
    USB_CFG_VENDOR_ID,  /* 2 bytes */
    USB_CFG_DEVICE_ID,  /* 2 bytes */
    USB_CFG_DEVICE_VERSION, /* 2 bytes */
    1,      /* manufacturer string index */
    2,      /* product string index */
    0,      /* serial number string index */
    1,      /* number of configurations */
};

// B.2 Configuration Descriptor
const PROGMEM char configDescrMIDI[] = {  /* USB configuration descriptor */
    9,      /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,  /* descriptor type */
    101, 0,     /* total length of data returned (including inlined descriptors) */
    2,      /* number of interfaces in this configuration */
    1,      /* index of this configuration */
    0,      /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    USBATTR_SELFPOWER,  /* attributes */
#else
    USBATTR_BUSPOWER, /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER,  /* max USB current in 2mA units */
    
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
    9,      /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,      /* index of this interface */
    0,      /* alternate setting for this interface */
    0,      /* endpoints excl 0: number of endpoint descriptors to follow */
    1,      /* */
    1,      /* */
    0,      /* */
    0,      /* string index for interface */
    
    // B.3.2 Class-specific AC Interface Descriptor
    // The Class-specific AC interface descriptor is always headed by a Header descriptor
    // that contains general information about the AudioControl interface. It contains all
    // the pointers needed to describe the Audio Interface Collection, associated with the
    // described audio function. Only the Header descriptor is present in this device
    // because it does not contain any audio functionality as such.
    /* AC Class-Specific descriptor */
    9,      /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    36,     /* descriptor type */
    1,      /* header functional descriptor */
    0x0, 0x01,    /* bcdADC */
    9, 0,     /* wTotalLength */
    1,      /* */
    1,      /* */
    
    // B.4 MIDIStreaming Interface Descriptors
    
    // B.4.1 Standard MS Interface Descriptor
    /* interface descriptor follows inline: */
    9,      /* length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    1,      /* index of this interface */
    0,      /* alternate setting for this interface */
    2,      /* endpoints excl 0: number of endpoint descriptors to follow */
    1,      /* AUDIO */
    3,      /* MS */
    0,      /* unused */
    0,      /* string index for interface */
    
    // B.4.2 Class-specific MS Interface Descriptor
    /* MS Class-Specific descriptor */
    7,      /* length of descriptor in bytes */
    36,     /* descriptor type */
    1,      /* header functional descriptor */
    0x0, 0x01,    /* bcdADC */
    65, 0,      /* wTotalLength */
    
    // B.4.3 MIDI IN Jack Descriptor
    6,      /* bLength */
    36,     /* descriptor type */
    2,      /* MIDI_IN_JACK desc subtype */
    1,      /* EMBEDDED bJackType */
    1,      /* bJackID */
    0,      /* iJack */
    
    6,      /* bLength */
    36,     /* descriptor type */
    2,      /* MIDI_IN_JACK desc subtype */
    2,      /* EXTERNAL bJackType */
    2,      /* bJackID */
    0,      /* iJack */
    
    //B.4.4 MIDI OUT Jack Descriptor
    9,      /* length of descriptor in bytes */
    36,     /* descriptor type */
    3,      /* MIDI_OUT_JACK descriptor */
    1,      /* EMBEDDED bJackType */
    3,      /* bJackID */
    1,      /* No of input pins */
    2,      /* BaSourceID */
    1,      /* BaSourcePin */
    0,      /* iJack */
    
    9,      /* bLength of descriptor in bytes */
    36,     /* bDescriptorType */
    3,      /* MIDI_OUT_JACK bDescriptorSubtype */
    2,      /* EXTERNAL bJackType */
    4,      /* bJackID */
    1,      /* bNrInputPins */
    1,      /* baSourceID (0) */
    1,      /* baSourcePin (0) */
    0,      /* iJack */
    
    
    // B.5 Bulk OUT Endpoint Descriptors
    
    //B.5.1 Standard Bulk OUT Endpoint Descriptor
    9,      /* bLenght */
    USBDESCR_ENDPOINT,  /* bDescriptorType = endpoint */
    0x1,      /* bEndpointAddress OUT endpoint number 1 */
    3,      /* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
    8, 0,     /* wMaxPacketSize */
    10,     /* bIntervall in ms */
    0,      /* bRefresh */
    0,      /* bSyncAddress */
    
    // B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
    5,      /* bLength of descriptor in bytes */
    37,     /* bDescriptorType */
    1,      /* bDescriptorSubtype */
    1,      /* bNumEmbMIDIJack  */
    1,      /* baAssocJackID (0) */
    
    
    //B.6 Bulk IN Endpoint Descriptors
    
    //B.6.1 Standard Bulk IN Endpoint Descriptor
    9,      /* bLenght */
    USBDESCR_ENDPOINT,  /* bDescriptorType = endpoint */
    0x81,     /* bEndpointAddress IN endpoint number 1 */
    3,      /* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
    8, 0,     /* wMaxPacketSize */
    10,     /* bIntervall in ms */
    0,      /* bRefresh */
    0,      /* bSyncAddress */
    
    // B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
    5,      /* bLength of descriptor in bytes */
    37,     /* bDescriptorType */
    1,      /* bDescriptorSubtype */
    1,      /* bNumEmbMIDIJack (0) */
    3,      /* baAssocJackID (0) */
};
    
    uchar usbFunctionDescriptor(usbRequest_t * rq)
    {
        
        if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
            usbMsgPtr = (uchar *) deviceDescrMIDI;
            return sizeof(deviceDescrMIDI);
        } else {    /* must be config descriptor */
            usbMsgPtr = (uchar *) configDescrMIDI;
            return sizeof(configDescrMIDI);
        }
    }
    uchar usbFunctionSetup(uchar data[8])
    {
        usbRequest_t    *rq = (usbRequest_t *)((void *)data);
        
        if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
            
            // Prepare bulk-in endpoint to respond to early termination
            if ((rq->bmRequestType & USBRQ_DIR_MASK) ==
                USBRQ_DIR_HOST_TO_DEVICE);
        }
        
        //We dont handle any data from the host.
        return 0; //0xff;
    }
    uchar usbFunctionRead(uchar * data, uchar len)
    {
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        data[4] = 0;
        data[5] = 0;
        data[6] = 0;
        
        return 7;
    }
    uchar usbFunctionWrite(uchar * data, uchar len)
    {
        return 1;
    }
    void usbFunctionWriteOut(uchar * data, uchar len)
    {
    }