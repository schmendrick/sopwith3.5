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
#ifndef SOPWITH_SDL_SOUND_H
#define SOPWITH_SDL_SOUND_H

#include <SDL/SDL.h>
#include "../sw_excep.h"
#include <cmath>

const unsigned int samplefrequency=22050;
const unsigned int constructionVolume=16;

float currentFreq;

void callback(void* /*userdata*/,Uint8* stream,int len)
{
  if (currentFreq==0)
    return;
  static int lastTime;
  static float lastFreq;

  lastTime*=static_cast<int>(lastFreq/currentFreq);

  unsigned char* buffer=new unsigned char[len];
  for (int i=0;i<len;++i) {
    float time=currentFreq*(i+lastTime);
    buffer[i]=time-static_cast<int>(time)<0.5f ? 0xff : 0;
  }
  SDL_MixAudio(stream,buffer,len,constructionVolume);
  delete[] buffer;

  lastTime+=len;
  lastFreq=currentFreq;
}

void initsound()
{
  std::cout<<"Initializing sound...";

  SDL_AudioSpec audioSpec;
  audioSpec.freq=samplefrequency;
  audioSpec.format=AUDIO_U8;
  audioSpec.channels=1;
  audioSpec.samples=1024;
  audioSpec.callback=callback;

  if (SDL_OpenAudio(&audioSpec,0)<0) {
    std::stringstream exception;
    exception << "Couldn't open audio: " << SDL_GetError();
    throw sw_excep(exception);
  }
  SDL_PauseAudio(0);

  std::cout<<"Done.\n";
}

void deinitsound()
{
  SDL_CloseAudio();
}

unsigned short lastfreq=0;
bool occupied=false;

void soundoff()
{
  if (lastfreq!=0) {
    SDL_PauseAudio(1);
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
  if (freq==37) {
    voice=0;
    soundoff();
    return true;
  }  
  if (voice==0 && occupied)
    return false;
  if (lastfreq!=freq) {
    currentFreq=clockTickRate/(static_cast<float>(freq)*samplefrequency);
    if (lastfreq==0)
      SDL_PauseAudio(0);
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

#endif /* SOPWITH_SDL_SOUND_H */
