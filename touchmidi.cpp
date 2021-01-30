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
/*----------------------------------------------------------------------------*/
TouchMIDI40::TouchMIDI40(void) :
  crntTouch{0}, tchNote{0}, swonCount(0),
  oct(0)
{}
/*----------------------------------------------------------------------------*/
void TouchMIDI40::mainLoop(int devNum)
{
  if (devNum == 3){
    int led1 = HIGH, led2 = HIGH;
    if (oct > 0){led1=LOW;}
    else if (oct < 0){led2=LOW;}
    digitalWrite(LED1, led1);
    digitalWrite(LED2, led2);
  }
}
/*----------------------------------------------------------------------------*/
void TouchMIDI40::checkTouch(uint16_t sw[])
{
  for (int i=0; i<4; ++i){
    for (int j=0; j<10; ++j){
      bool flg=false;
      if ((sw[i] >> j) & 0x0001){flg=true;}
      if (flg^crntTouch[i*10+j]){makeNoteEvent(i*10+j+40, flg);}
      crntTouch[i*10+j] = flg;
    }
  } 
}
/*----------------------------------------------------------------------------*/
void TouchMIDI40::checkTouch3dev(uint16_t sw[])
{
  int NTT[30] = { 25,  0,  1,  2,  3,  4,  5,  6,  7,  8, 
                   9, 10, 11, 26, 27, 28, 12, 13, 14, 15,
                  16, 17, 18, 19, 20, 21, 22, 23, 24, 29 };
  
  for (int i=0; i<3; ++i){
    for (int j=0; j<10; ++j){
      bool flg=false;
      int num = i*10+j;
      if ((sw[i] >> j) & 0x0001){flg=true;}
      if (flg^crntTouch[num]){
        if (NTT[num] < 25){
          int midiNote;
          if (flg){ tchNote[NTT[num]] = midiNote = NTT[num] + 48 + oct*12;}
          else { midiNote = tchNote[NTT[num]];}
          makeNoteEvent(midiNote, flg);
        }
        else if (flg){
          switch(NTT[num]){
            case 25: oct--; if(oct<-2) oct=-2; break;
            case 29: oct++; if(oct>2) oct=2; break;
            case 26:
            case 27:
            case 28:
            default: break;
          }
        }
      }
      crntTouch[num] = flg;
    }
  } 
}
/*----------------------------------------------------------------------------*/
void TouchMIDI40::makeNoteEvent(int notenum, bool onoff, int vel=127)
{
  if (onoff){
    // Note On
    setMidiNoteOn(notenum,vel);
    swonCount++;
  }
  else {
    //  Note Off
    setMidiNoteOff(notenum,0);
    swonCount--;
    if ( swonCount <= 0 ){
      swonCount = 0;
    }
  }
}
