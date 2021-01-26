/* ========================================
 *
 *  TouchMIDI Common Platform for AVR
 *  TouchMIDI.cpp
 *    description: TouchMIDI40
 *
 *  Copyright(c)2021- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#include "touchmidi.h"
#include "TouchMIDI_AVR_if.h"
#include "configuration.h"
#include "i2cdevice.h"

TouchMIDI40::TouchMIDI40(void) : crntTouch{0}, swonCount(0)
{}

void TouchMIDI40::mainLoop(void)
{}

void TouchMIDI40::checkTouch(uint8_t sw[][2])
{
  for (int i=0; i<4; ++i){
    for (int j=0; j<8; ++j){
      bool flg=false;
      if ((sw[i][0] >> i) & 0x01){flg=true;}
      if (flg^crntTouch[i*10+j]){makeMIDIEvent(i*10+j);}
      crntTouch[i*10+j] = flg;
    }
    for (int k=0; k<2; ++k){
      bool flg=false;
      if ((sw[i][1] >> i) & 0x01){flg=true;}
      if (flg^crntTouch[i*10+8+k]){makeMIDIEvent(i*10+8+k);}
      crntTouch[i*10+8+k] = flg;
    }
  } 
}

void TouchMIDI40::makeMIDIEvent(int tchNum)
{
  if ( crntTouch[tchNum] == false ){
    // Note On
    setMidiNoteOn(tchNum+40,127);
    digitalWrite(LED1, HIGH);
    swonCount++;
  }
  else {
    //  Note Off
    setMidiNoteOff(tchNum+40,0);
    if ( swonCount != 0 ){ swonCount--;}
    if ( swonCount == 0 ){
      digitalWrite(LED1, LOW);
    }
  }
}
