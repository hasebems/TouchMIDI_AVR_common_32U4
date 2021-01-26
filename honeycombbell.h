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
#ifndef HONEYCOMBBELL_H
#define HONEYCOMBBELL_H

#include <Arduino.h>

/*----------------------------------------------------------------------------*/
static const int _MAX_SET_NUMBER = 8;
static const int _MAX_BEAT = 8;
static const int _MAX_LED_PER_DEVICE = 6; //  device means one CY8CMBR3110
static const int _MAX_DEVICE_NUM = 2;
static const int _MAX_LED = _MAX_LED_PER_DEVICE*_MAX_DEVICE_NUM;
/*----------------------------------------------------------------------------*/
enum LED_STATE {
  LIGHT_OFF,
  WHITE_ON,
  TOUCH_ON,
  FADE_OUT
};
/*----------------------------------------------------------------------------*/
class EachLed {

public:  
  EachLed( void ) : _myLocate(0), _fadeCounter(0) {}

  void checkFade( void );

  void setLocate( int locate ){ _myLocate = locate;}
  void setNeoPixel( LED_STATE sw );

private:
  static const int _FADE_TIME = 20;

  int   _myLocate;
  int   _fadeCounter;   // 0:LIGHT_OFF, _FADE_TIME:TOUCH_ON

};
/*----------------------------------------------------------------------------*/
class HoneycombBell {

public:
  HoneycombBell( void );

  void mainLoop( void );
  void periodic100msec( void ){}
  void rcvClock( uint8_t msg );
  void checkTwelveTouch( int device );
  void decideOctave( void );

  void setSetNumber( int num ){ _setNumber = num;}

private:
  void displayNextBeat( void );

  uint16_t  _swState[2];
  int       _octave;
  int       _tempo;   //  times*10msec  50:120bpm, 20:300bpm
  uint32_t  _nextGlbTm;

  int       _beat;    //  0 - 7
  int       _setNumber; //  0 - 7
  int       _myConnectionNumber;  //  0 - 15

  EachLed   _led[_MAX_LED];
};
#endif
