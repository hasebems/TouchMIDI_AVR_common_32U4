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
  void mainLoop(int devNum);
  void checkTouch(uint16_t sw[]);
  void checkTouch3dev(uint16_t sw[]);
  void makeNoteEvent(int tchNum, bool onoff, int vel=127);

private:
  bool  crntTouch[40];
  int   tchNote[25];    // for 25key
  int   swonCount;
  int   oct;            // for 25key
  int   velocity;       // for 25key

};
#endif
