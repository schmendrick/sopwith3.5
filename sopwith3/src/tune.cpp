/*
    Sopwith 3
    Copyright (C) 1984-2000  David L. Clark
    Copyright (C) 1999-2001  Andrew Jenner
    Copyright (C) 2001-2003  Jornand de Buisonjé

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef SOPWITH_TUNE_CPP
#define SOPWITH_TUNE_CPP

#include "tune.h"
#include "def.h"
#include <sstream>

Tune::Tune(const std::string& s,unsigned int i) :
  composition(s),
  tempo(i),
  ticks(0)
{
}

void Tune::playnote()
{
  static const int majorscale[7]={0,2,3,5,7,8,10};
  static const int notefreq[12]={440,466,494,523,554,587,622,659,698,740,784,831};
  std::stringstream durbuf;

  int sharpen=0,dotdur=2,noteoctave=0x100,semitone=0,duration=4;
  unsigned short freq;
  char tunechar,noteletter='R';
  do {
    if (place==0)
      octavemultiplier=0x100;
    tunechar=toupper(composition[place++]);
    if (place==composition.length()) {
      place=0;
    }
    else {
      if (isalpha(tunechar)) {
        semitone=majorscale[tunechar-'A'];
        noteletter=tunechar;
      }
      else
        switch(tunechar) {
          case '>':
            octavemultiplier<<=1;
            break;
          case '<':
            octavemultiplier>>=1;
            break;
          case '+':
            ++sharpen;
            break;
          case '-':
            --sharpen;
            break;
          case '.':
            dotdur=3;
            break;
          default:
            if (isdigit(tunechar))
              durbuf<<tunechar;
        }
    }
  } while (tunechar!='/');
  durbuf>>duration;
  if (duration<=0)
    duration=4;
  if (noteletter=='R')
    freq=0x7d00;
  else {
    semitone+=sharpen;
    while (semitone<0) {
      semitone+=12;
      noteoctave>>=1;
    }
    while (semitone>=12) {
      semitone-=12;
      noteoctave<<=1;
    }
    freq=static_cast<short>((static_cast<long>(notefreq[semitone])*static_cast<long>(octavemultiplier)*
                  static_cast<long>(noteoctave))>>16);
  }
  tone=static_cast<short>(clockTickRate/freq);
  ticks+=((dotdur*tempo)/(duration*60))>>1;
}

#endif /* SOPWITH_TUNE_CPP */
