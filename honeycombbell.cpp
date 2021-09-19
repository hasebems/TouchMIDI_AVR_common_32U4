/* ========================================
 *
 *  TouchMIDI Common Platform for AVR
 *  honeycombbell.h
 *    description: HoneycombBell 
 *
 *  Copyright(c)2019- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#include "honeycombbell.h"
#include "TouchMIDI_AVR_if.h"
#include "configuration.h"
#include "i2cdevice.h"
#include <avr/pgmspace.h>

/*----------------------------------------------------------------------------*/
extern GlobalTimer gt;

/*----------------------------------------------------------------------------*/
HoneycombBell::HoneycombBell( void ) : _swState(), _octave(0),
                          _tempo(20), _nextGlbTm(20), _beat(0),
                          _setNumber(0), _myConnectionNumber(0)
{
  for ( int i=0; i<_MAX_LED; ++i ){ _led[i].setLocate(i);}
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::mainLoop( void )
{
  if ( gt.timer10msecEvent() ){
    //  LED Fade Out
    for ( int i=0; i<_MAX_LED; i++ ){
      _led[i].checkFade();
    }

    // only first board
    if (( _myConnectionNumber == 0) && ( _nextGlbTm < gt.timer10ms() )){
      displayNextBeat();
      midiClock(_beat<<4);
      _beat += 1;
      if ( _beat == _MAX_BEAT ){ _beat = 0;}
      _nextGlbTm += _tempo;
    }
  }

  if ( gt.timer100msecEvent() ){
    
  }
}
/*------------------------------------------------------------------------------
[[ Board Connection Placement & Order ]]
Set Number 1: [ 1, 0 ]
Set Number 2: [ 3, 2 ]
              [ 0, 1 ]
Set Number 3: [ 5, 4, 3 ]
              [ 0, 1, 2 ]
Set Number 4: [ 8, 7, 6 ]
              [ 3, 4, 5 ]
              [ 2, 1, 0 ]
Set Number 5: [ 11, 10, 9, 8 ]
              [ 4, 5, 6, 7 ]
              [ 3, 2, 1, 0 ]
Set Number 6: [ 15, 14, 13, 12 ]
              [ 8, 9, 10, 11 ]
              [ 7, 6, 5, 4 ]
              [ 0, 1, 2, 3 ]
------------------------------------------------------------------------------*/
const uint8_t OctTable[8][16] PROGMEM = {
//  0,1,2,3, 4,5,6,7, 8,9,10,11,12,13,14,15
  { 6,5,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0},  
  { 6,5,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0},  //  Set Number 1: Basic
  { 5,6,6,5, 0,0,0,0, 0,0,0,0,  0,0,0,0},  //  2:    2by2
  { 5,6,7,7, 6,5,0,0, 0,0,0,0,  0,0,0,0},  //  3:    2by3
  { 7,6,5,5, 6,7,7,6, 5,0,0,0,  0,0,0,0},  //  4:    3by3
  { 7,6,5,4, 4,5,6,7, 7,6,5,4,  0,0,0,0},  //  5:    3by4
  { 7,6,5,4, 4,5,6,7, 7,6,5,4,  4,5,6,7},  //  6:    4by4
  { 6,5,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0}   //  
};
void HoneycombBell::decideOctave( void )
{
  _octave = pgm_read_byte_near(OctTable[_setNumber] + _myConnectionNumber);
}
/*----------------------------------------------------------------------------*/
//  This function is called when MIDI CC#10h comes to this board.
void HoneycombBell::rcvClock( uint8_t msg )
{
  int sentNum = msg & 0x0f;
  int beatNum = (msg & 0x70)>>4;

  if ( _myConnectionNumber != sentNum + 1 ){
    _myConnectionNumber = sentNum + 1;
    decideOctave();
  }
  _beat = beatNum;
  displayNextBeat();
  midiClock((msg & 0x70) | _myConnectionNumber);
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::checkTwelveTouch(uint16_t sw[])
{
  for (int dev=0; dev<_MAX_DEVICE_NUM; dev++){
    const int baseNum = dev*_MAX_LED_PER_DEVICE;

    for ( int i=0; i<_MAX_LED_PER_DEVICE; i++ ){
      uint16_t  bitPtn = 0x0001 << i;
      if ( (_swState[dev]&bitPtn)^(sw[dev]&bitPtn) ){
        if ( sw[dev] & bitPtn ){
          setMidiNoteOn( baseNum+i+(12*_octave), 0x7f );
          _led[baseNum+i].setNeoPixel( TOUCH_ON );
        }
        else {
          setMidiNoteOff( baseNum+i+(12*_octave), 0x40 );
          _led[baseNum+i].setNeoPixel( FADE_OUT );
        }
      }
    }
    _swState[dev] = sw[dev];
    //setAda88_Number((swState[0]<<6) | (swState[1]&0x3f));
  }
}

/*----------------------------------------------------------------------------*/
//  How to use
//  1.Think LED pattern for non-playing state.
//  2.Analyse light pattern of each block / each beat.
//  3.Describe each pattern to Table "EachLedPattern". And note pattern number.
//  4.Describe pattern number to Table "LedWaitPtnNumber".
//    1st Index means setNumber. 2nd Index means block number by connected order.

const uint8_t EachLedPattern[6][_MAX_LED] PROGMEM = {
  //  Value means bit0:beat0, bit1:beat1 .... bit7:beat7
  { 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10 }, // Pattern 0
  { 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01 }, // Pattern 1
  { 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08 }, // Pattern 2
  { 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80 }, // Pattern 3
  { 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04 }, // Pattern 4
  { 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40 }  // Pattern 5
};
const uint8_t LedWaitPtnNumber[8][16] PROGMEM = {
// Select Each Led Pattern above EachLedPattern[x]
// 1st2nd3rd...(block)
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  not use
  { 1, 0, /* */0,0,0,0,0,0,0,0,0,0,0,0,0,0 },         //  setNumber = 1
  { 2, 3, 1, 0, /* */0,0,0,0,0,0,0,0,0,0,0,0 },       //  setNumber = 2
  { 2, 3, 2, 0, 1, 0, /* */0,0,0,0,0,0,0,0,0,0 },     //  setNumber = 3
  { 4, 5, 4, 2, 3, 2, 0, 1, 0, /* */0,0,0,0,0,0,0 },  //  setNumber = 4
  { 5, 4, 5, 4, 3, 2, 3, 2, 1, 0, 1, 0, /* */0,0,0,0 },//  setNumber = 5
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 6
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  //  not use
};
/*----------------------------------------------------------------------------*/
void HoneycombBell::displayNextBeat( void )
{
  const uint8_t beatBit = 0x80 >> _beat;
  const uint8_t waitPtn = pgm_read_byte_near(LedWaitPtnNumber[_setNumber]+_myConnectionNumber);
  
  for ( int i=0; i<_MAX_LED; i++ ){
    const uint8_t ptn = pgm_read_byte_near(EachLedPattern[waitPtn]+i);
    if ( ptn & beatBit ){
      _led[i].setNeoPixel( WHITE_ON );
    }
    else {
      _led[i].setNeoPixel( LIGHT_OFF );      
    }
  }
}


/*----------------------------------------------------------------------------*/
void EachLed::checkFade( void )
{
  if (( _fadeCounter > 0 ) && ( _fadeCounter < _FADE_TIME )){
    _fadeCounter -= 1;

    const uint8_t red = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,0)*_fadeCounter)/_FADE_TIME);
    const uint8_t blu = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,1)*_fadeCounter)/_FADE_TIME);
    const uint8_t grn = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,2)*_fadeCounter)/_FADE_TIME);
  
    setLed(_myLocate,red,blu,grn);
    lightLed();
  }   
}
/*----------------------------------------------------------------------------*/
void EachLed::setNeoPixel( LED_STATE sw )
{
  if ( sw == TOUCH_ON ){
    setLed(_myLocate,colorTbl(_myLocate%16,0),colorTbl(_myLocate%16,1),colorTbl(_myLocate%16,2));
    _fadeCounter = _FADE_TIME;
  }
  else if ( sw == FADE_OUT ){
    if ( _fadeCounter > 0 ){
      _fadeCounter -= 1;
    }
  }
  if ( _fadeCounter == 0 ){
    if ( sw == LIGHT_OFF ){
      setLed(_myLocate,0,0,0);
    }
    else if ( sw == WHITE_ON ){
      setLed(_myLocate,100,100,100);
    }
  }
  lightLed();
}
