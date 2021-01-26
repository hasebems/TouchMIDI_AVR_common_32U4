/* ========================================
 *
 *  TouchMIDI Common Platform for AVR
 *  touchmidi.h
 *    description: TouchMIDI40 
 *
 *  Copyright(c)2021- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#ifndef TOUCHMIDI40_H
#define TOUCHMIDI40_H

#include <Arduino.h>

class TouchMIDI40 {

public:
  TouchMIDI40(void);
  void mainLoop(void);
  void checkTouch(uint8_t sw[][2]);
  void makeMIDIEvent(int tchNum);

private:
  bool  crntTouch[40];
  int   swonCount;

};
#endif
