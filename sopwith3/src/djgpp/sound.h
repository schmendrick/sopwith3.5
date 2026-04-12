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
#ifndef SOPWITH_DJGPP_SOUND_H
#define SOPWITH_DJGPP_SOUND_H

#include <pc.h>
#include "../soundsys.h"

void initsound()
{
}

void deinitsound()
{
  soundoff();
}

unsigned short lastfreq=0;
bool occupied=false;

void soundoff()
{
  if (lastfreq!=0) {
    outportb(0x61,inportb(0x61)&0xfc);
    lastfreq=0;
    occupied=false;
  }
}

void deletevoice(void* voice)
{
  if (voice!=0)
    occupied=false;
}

bool tone(unsigned short freq,int /*soundpriority*/,void*& voice)
{
  if (voice==0 && occupied)
    return false;
  if (lastfreq!=freq) {
    if (lastfreq==0)
      outportb(0x43,0xb6);
    outportb(0x42,freq);
    outportb(0x42,freq>>8);
    if (lastfreq==0)
      outportb(0x61,inportb(0x61)|3);
    lastfreq=freq;
  }
  occupied=true;
  voice=reinterpret_cast<void*>(&occupied); // or &lastfreq
  return true;
}

void finalizesound()
{
  if (occupied==false)
    soundoff();
}

void* sample(Soundevent /*soundevent*/)
{
  return 0;
}

bool playsoundevent(void* /*eventvoice*/)
{
  return false;
}

bool playtheme()
{
  return false;
}

#endif /* SOPWITH_DJGPP_SOUND_H */
