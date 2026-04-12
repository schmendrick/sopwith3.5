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
#ifndef SOPWITH_ALLEGRO_SOUND_H
#define SOPWITH_ALLEGRO_SOUND_H

#include <cmath>
#include <allegro.h>
#include "../def.h"
#include "../soundsys.h"
#include <map>

SAMPLE* speaker=0;
const unsigned int samplefrequency=22050;
MIDI* theme=0;
std::map<int,SAMPLE*> samples;
std::map<int,unsigned short> normalfreqs;
SAMPLE* soundevents[SOUND_EVENT_SIZE]={0};

void initsound()
{
  install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,0);
  theme=load_midi("data/music/theme.mid");

  samples[SOUND_PRIORITY_FIRING        ]=load_sample("data/sounds/planfire.wav");
  samples[SOUND_PRIORITY_PLANE_OK      ]=load_sample("data/sounds/planengi.wav");
  samples[SOUND_PRIORITY_PLANE_TROUBLED]=load_sample("data/sounds/planengi.wav");
  normalfreqs[SOUND_PRIORITY_PLANE_OK      ]=movingplanebasesound-(MIN_SPEED+MAX_THROTTLE)*planespeedsoundmultiplier;
  normalfreqs[SOUND_PRIORITY_PLANE_TROUBLED]=movingplanebasesound-(MIN_SPEED+MAX_THROTTLE)*planespeedsoundmultiplier;
  soundevents[  BOMB_DESTROYED]=load_sample("data/sounds/bombexpl.wav");
  soundevents[TARGET_DESTROYED]=load_sample("data/sounds/targexpl.wav");
  soundevents[TARGET_FIRED    ]=load_sample("data/sounds/targfire.wav");
}

void deinitsound()
{
  soundoff();
  if (theme!=0)
    destroy_midi(theme);

  const std::map<int,SAMPLE*>::const_iterator end=samples.end();
  for (std::map<int,SAMPLE*>::iterator sample=samples.begin();sample!=end;++sample)
    destroy_sample(sample->second);
  samples.clear();
  for (int i=0;i<SOUND_EVENT_SIZE;++i) {
    if (soundevents[i]!=0) {
      destroy_sample(soundevents[i]);
      soundevents[i]=0;
    }
  }
  remove_sound();
}

void soundoff()
{
  for (int i=0;i<SOUND_EVENT_SIZE;++i) {
    if (soundevents[i]!=0)
      stop_sample(soundevents[i]);
  }
  stop_midi();
}

class Voice {
public:
  Voice(int voicenumber,SAMPLE* sample,bool destroysample,unsigned short freq) : voicenumber(voicenumber),sample(sample),destroysample(destroysample),freq(freq)
  {
    voice_start(voicenumber);
  }
  ~Voice()
  {
    deallocate_voice(voicenumber);
    if (destroysample)
      destroy_sample(sample);
  }
  int voicenumber;
  SAMPLE* sample;
  bool destroysample;
  unsigned short freq;
};

void deletevoice(void* voice)
{
  delete reinterpret_cast<Voice*>(voice);
}

bool tone(unsigned short freq,int soundpriority,void*& voice)
{
  Voice*& voice_=reinterpret_cast<Voice*>(voice);
  SAMPLE* sample=samples[soundpriority];
  if (freq==0)
    freq=0xffff;
  if (voice!=0 && voice_check(voice_->voicenumber)==0) {
    delete voice_;
    voice=0;
  }
  if (voice!=0 && freq==voice_->freq && sample==voice_->sample)
    return true;
  if (sample!=0) {
    if (voice!=0) {
      if (sample==voice_->sample) {
        unsigned short normalfreq=normalfreqs[soundpriority];
        if (normalfreq>0) {
          voice_set_frequency(voice_->voicenumber,static_cast<short>(static_cast<float>(sample->freq)*normalfreq/freq));
          voice_->freq=freq;
        }
        return true;
      }
      else {
        delete voice_;
        voice=0;
      }
    }
    int voicenumber=allocate_voice(sample);
    if (voicenumber==-1)
      return false;
    voice_set_volume(voicenumber,64);
    unsigned short normalfreq=normalfreqs[soundpriority];
    if (normalfreq>0)
       voice_set_frequency(voicenumber,static_cast<short>(static_cast<float>(sample->freq)*normalfreq/freq));
    else
       voice_set_frequency(voicenumber,sample->freq);
    voice_set_playmode(voicenumber,PLAYMODE_LOOP);
    voice=new Voice(voicenumber,sample,false,freq);
    return true;
  }

  if (voice!=0) {
    if (freq==37) {
      delete voice_;
      voice=0;
      return true;
    }
    if (std::abs(static_cast<short>(freq-(voice_->freq)))<=8) {
      SAMPLE* sample=voice_->sample;
      voice_set_frequency(voice_->voicenumber,static_cast<short>(static_cast<double>(clockTickRate)*sample->freq*sample->len/(static_cast<float>(freq)*samplefrequency)));
      voice_->freq=freq;
      return true;
    }
    else {
      delete voice_;
      voice=0;
    }
  }
  else
    if (freq==37)
      return true;
  float targetfreq=static_cast<float>(freq)*samplefrequency/clockTickRate;
  int basefreq=static_cast<short>(targetfreq)/2*2;
  SAMPLE* speaker=create_sample(8,FALSE,samplefrequency,basefreq);
  unsigned int length=speaker->len/2;
  memset(                                 speaker->data        ,0xff,length);
  memset(reinterpret_cast<unsigned char*>(speaker->data)+length,0x00,length);
  int voicenumber=allocate_voice(speaker);
  if (voicenumber==-1)
    return false;
  voice_set_volume(voicenumber,32);
  voice_set_frequency(voicenumber,static_cast<short>(static_cast<float>(speaker->freq)*basefreq/targetfreq));
  voice_set_playmode(voicenumber,PLAYMODE_LOOP);
  voice=new Voice(voicenumber,speaker,true,freq);
  return true;
}

void finalizesound()
{
}

void* sample(Soundevent soundevent)
{
  return soundevents[soundevent];
}

bool playsoundevent(void* eventvoice)
{
  SAMPLE* sample=reinterpret_cast<SAMPLE*>(eventvoice);
  return play_sample(sample,128,128,1000,FALSE)>=0;
}

bool playtheme()
{
  return (theme!=0) && (play_midi(theme,TRUE)==0);
}

#endif /* SOPWITH_ALLEGRO_SOUND_H */
