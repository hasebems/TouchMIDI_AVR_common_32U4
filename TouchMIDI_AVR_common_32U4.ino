/* ========================================
 *
 *  TouchMIDI_AVR_common_32U4
 *    description: Main Loop
 *    for Arduino Leonardo
 *
 *  Copyright(c)2021- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#include  <MsTimer2.h>
#include  <Adafruit_NeoPixel.h>
#include  <MIDI.h>
#include  <MIDIUSB.h>

#include  "configuration.h"
#include  "TouchMIDI_AVR_if.h"

#include  "i2cdevice.h"
#include  "honeycombbell.h"
#include  "touchmidi.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

/*----------------------------------------------------------------------------*/
//
//     Global Variables
//
/*----------------------------------------------------------------------------*/
Adafruit_NeoPixel led = Adafruit_NeoPixel(MAX_LED, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
MIDI_CREATE_DEFAULT_INSTANCE();

/*----------------------------------------------------------------------------*/
int maxCapSenseDevice;
bool availableEachDevice[4];

GlobalTimer gt;
static HoneycombBell hcb;
static TouchMIDI40 tm40;

/*----------------------------------------------------------------------------*/
void flash()
{
  gt.incGlobalTime();
}
/*----------------------------------------------------------------------------*/
void setup()
{
  int err;
  int i;

  //  Initialize Hardware
  wireBegin();
//  Serial.begin(31250);
  MIDI.setHandleNoteOff( handlerNoteOff );
  MIDI.setHandleNoteOn( handlerNoteOn );
  MIDI.setHandleControlChange( handlerCC );
  MIDI.begin();
  MIDI.turnThruOff();

#ifdef USE_ADA88
  ada88_init();
  ada88_write(0);
#endif

  //  Read Jumper Pin Setting
  pinMode(MODEPIN1, INPUT); 
  pinMode(MODEPIN2, INPUT);

  pinMode(LED1, OUTPUT);   // LED1
  digitalWrite(LED1, LOW);
//  pinMode(LED2, OUTPUT);   // LED2
//  digitalWrite(LED2, HIGH);
  
  const uint8_t md1 = digitalRead(MODEPIN1);
  const uint8_t md2 = digitalRead(MODEPIN2);

  for (i=0; i<4; ++i){
    availableEachDevice[i] = true;
  }

  //  set mode
  if (( md1 == LOW ) && ( md2 == LOW )){
    maxCapSenseDevice = 4;  //  TouchMIDI40
  }
  else if (( md1 == HIGH ) && ( md2 == LOW )){
    maxCapSenseDevice = 3;  //  TouchMIDI30key
  }
  else if (( md1 == LOW ) && ( md2 == HIGH )){
    maxCapSenseDevice = 2;  //  HoneycombBell
  }
  else {  //  HIGH-HIGH
    maxCapSenseDevice = 1;  //  WindMIDI
  }

#if 1 // CapSense Setup Mode
  int errNum = 0;
  for (i=0; i<maxCapSenseDevice; ++i){
    err = MBR3110_setup(i);
    if (err){
      availableEachDevice[i] = false;
      digitalWrite(LED1, HIGH);
      errNum += 0x01<<i;
    }
  }
  setAda88_Number(errNum*10);
  delay(2000);
  digitalWrite(LED1, LOW);
#endif

  //  Normal Mode
  MBR3110_resetAll(maxCapSenseDevice);
  errNum = 0;
  for (i=0; i<maxCapSenseDevice; ++i){
    if (availableEachDevice[i]){
      err = MBR3110_init(i);
      if (err){
        availableEachDevice[i] = false;       
        errNum += 0x01<<i;
      }
    }
  }
  if (errNum){
    //  if err, stop 5sec.
    digitalWrite(LED1, HIGH);
    setAda88_Number(errNum*10);
    delay(5000);
    digitalWrite(LED1, LOW);
    ada88_write(0);
  }

  //  Set NeoPixel Library 
  led.begin();
  led.show(); // Initialize all pixels to 'off'

  //  Set Interrupt
  MsTimer2::set(10, flash);     // 10ms Interval Timer Interrupt
  MsTimer2::start();
}
/*----------------------------------------------------------------------------*/
void loop()
{  
  //  Global Timer 
  generateTimer();

  //  Application
  switch(maxCapSenseDevice){
    case 2: hcb.mainLoop(); break;
    case 3: break;
    case 4: tm40.mainLoop(); break;
    default: break;
  }

  //  MIDI Receive
  receiveMidi();

  if ( gt.timer10msecEvent() ){
    //  Touch Sensor
    uint8_t sw[MAX_DEVICE_MBR3110][2] = {0};
 #ifdef USE_CY8CMBR3110
    int errNum = 0;
    for (int i=0; i<maxCapSenseDevice; ++i){
      if (availableEachDevice[i] == true){
        int err = MBR3110_readTouchSw(sw[i],i);
        if (err){
          errNum += 0x01<<i;          
        }
      }
    }
    if (errNum){
      setAda88_Number(errNum*10);
      digitalWrite(LED1, HIGH);
    }
    else {
      digitalWrite(LED1, LOW);
    }
 #endif
    switch(maxCapSenseDevice){
      case 3:
      case 4: tm40.checkTouch(sw); break;
      case 2: hcb.checkTwelveTouch(0); break;
      default: break;
    }
  }
}
/*----------------------------------------------------------------------------*/
//
//     Global Timer
//
/*----------------------------------------------------------------------------*/
void generateTimer( void )
{
  uint32_t diff = gt.readGlobalTimeAndClear();

  gt.clearAllTimerEvent();
  gt.updateTimer(diff);
  //setAda88_Number(diff);

  if ( gt.timer100msecEvent() == true ){
    //  heatbeat for Debug
    //(gt.timer100ms() & 0x0002)? digitalWrite(LED2, HIGH):digitalWrite(LED2, LOW);
    setAda88_Number(gt.timer100ms());
  }
}
/*----------------------------------------------------------------------------*/
//
//     MIDI Command & UI
//
/*----------------------------------------------------------------------------*/
void receiveMidi( void ){ MIDI.read();}
/*----------------------------------------------------------------------------*/
void setMidiNoteOn( uint8_t dt0, uint8_t dt1 )
{
//  uint8_t dt[3] = { 0x90, dt0, dt1 };
//  Serial.write(dt,3);
  MIDI.sendNoteOn( dt0, dt1, 1 );

  midiEventPacket_t event = {0x09, 0x90, dt0, dt1};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
/*----------------------------------------------------------------------------*/
void setMidiNoteOff( uint8_t dt0, uint8_t dt1 )
{
//  uint8_t dt[3] = { 0x80, dt0, dt1 };
//  Serial.write(dt,3);
  MIDI.sendNoteOff( dt0, dt1, 1 );

  midiEventPacket_t event = {0x09, 0x90, dt0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
/*----------------------------------------------------------------------------*/
void handlerNoteOn( byte channel , byte number , byte value ){ setMidiNoteOn( number, value );}
/*----------------------------------------------------------------------------*/
void handlerNoteOff( byte channel , byte number , byte value ){ setMidiNoteOff( number, value );}
/*----------------------------------------------------------------------------*/
void handlerCC( byte channel , byte number , byte value )
{
  if ( number == /*0x10*/ midi::GeneralPurposeController1 ){
    hcb.rcvClock( value );
  }
}
/*----------------------------------------------------------------------------*/
void midiClock( uint8_t msg )
{
//  uint8_t dt[3] = { 0xb0, 0x10, msg };

//  if ( isMasterBoard == false ){
//  Serial.write(dt,3);
//    MIDI.sendControlChange( midi::GeneralPurposeController1, msg, 1 );
//  }
}
/*----------------------------------------------------------------------------*/
//
//     Hardware Access Functions
//
/*----------------------------------------------------------------------------*/
void setAda88_Number( int number )
{
#ifdef USE_ADA88
  ada88_writeNumber(number);  // -1999 - 1999
#endif
}
/*----------------------------------------------------------------------------*/
//
//     Blink LED by NeoPixel Library
//
/*----------------------------------------------------------------------------*/
const uint8_t colorTable[16][3] = {
  { 200,   0,   0 },//  C
  { 175,  30,   0 },
  { 155,  50,   0 },//  D
  { 135,  70,   0 },
  { 110,  90,   0 },//  E
  {   0, 160,   0 },//  F
  {   0, 100, 100 },
  {   0,   0, 250 },//  G
  {  30,   0, 230 },
  {  60,   0, 190 },//  A
  { 100,   0, 140 },
  { 140,   0,  80 },//  B

  { 100, 100, 100 },
  { 100, 100, 100 },
  { 100, 100, 100 },
  { 100, 100, 100 }
 };
/*----------------------------------------------------------------------------*/
uint8_t colorTbl( uint8_t doremi, uint8_t rgb ){ return colorTable[doremi][rgb];}
void setLed( int ledNum, uint8_t red, uint8_t green, uint8_t blue )
{
  led.setPixelColor(ledNum,led.Color(red, green, blue));
}
void lightLed( void )
{
  led.show();
}
