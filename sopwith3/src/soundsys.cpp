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
#ifndef SOPWITH_SOUNDSYS_CPP
#define SOPWITH_SOUNDSYS_CPP

#include "soundsys.h"
#include "message.h"
#include <set>
#include <map>

const int leastsoundsubpriority=0x7fff;
const unsigned int stationaryplanesound=0xf000;
const unsigned int movingplanebasesound=0xd000;
const unsigned int planespeedsoundmultiplier=0x1000;
const unsigned int firingsound=0x1000;
const unsigned int troubledplanesound[]={0x9000,0xf000};
const unsigned int firingplanesound=0xf000;
const std::string sopwith2tune=
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./"
  "e4/g8/g2/r16/>a8/<g8/e8/d2./"
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./"
  "e4/>a8/a2/r16/<g8/f+8/e8/d2./"
  "d8/g2/r16/g8/g+2/r16/g+8/>a2/r16/a8/c2/r16/"
  "b8/a8/<g8/>b4/<g8/>b4/<g8/>a4./<g1/";
const unsigned int tempo=1440;
int soundticks;
int numfrags;
Musicmode titlemode=NO_MUSIC;
Tune title(sopwith2tune,tempo);
Tune explosion(sopwith2tune,tempo);

class Soundpriority {
public:
  Soundpriority(int soundpriority,int soundsubpriority,int rank)
  : soundpriority(soundpriority)
  , soundsubpriority(soundsubpriority)
  , rank(rank)
  {
  }
  inline bool operator<(const Soundpriority& rhs) const
  {
    return soundpriority< rhs.soundpriority ||
          (soundpriority==rhs.soundpriority && soundsubpriority< rhs.soundsubpriority) ||
          (soundpriority==rhs.soundpriority && soundsubpriority==rhs.soundsubpriority && rank<rhs.rank);
  }
  inline bool operator>(const Soundpriority& rhs) const
  {
    return soundpriority> rhs.soundpriority ||
          (soundpriority==rhs.soundpriority && soundsubpriority> rhs.soundsubpriority) ||
          (soundpriority==rhs.soundpriority && soundsubpriority==rhs.soundsubpriority && rank>rhs.rank);
  }
  int soundpriority,soundsubpriority,rank;
};

typedef std::pair<Object*,Soundpriority> Soundrequest;

class Sound {
public:
  Sound(const Soundrequest& sr) : sr(sr),lastfreq(0),voice(0) {}
  virtual ~Sound()
  {
    if (voice!=0)
      deletevoice(voice);
  }
  virtual bool tone(unsigned short freq)
  {
    lastfreq=freq;
    if (soundflag)
      return ::tone(freq,sr.second.soundpriority,voice);
    else
      return false;
  }
  virtual void soundadj() {};
public:
  Soundrequest sr;
  unsigned short lastfreq;
protected:
  void* voice;
};

class Sound_slide : public Sound {
public:
  Sound_slide(const Soundrequest& sr) : Sound(sr) {}
  virtual bool tone(unsigned short freq)
  {
    lastfreq=freq;
    if (soundflag)
      if (voice==0)
        return ::tone(freq,sr.second.soundpriority,voice);
      else
        return true;
    else
      return false;
  }
  virtual void soundadj()
  {
    if (sr.first!=0 && soundflag)
      ::tone(sr.first->tonefreq+sr.first->tonechange*soundticks,sr.second.soundpriority,voice);
  }
};

class Sound_shot : public Sound {
public:
  Sound_shot(const Soundrequest& sr) : Sound(sr) {}
  virtual void soundadj()
  {
    static unsigned short savefreq=0;
    if (lastfreq==firingplanesound) {
      if (soundflag)
        ::tone(savefreq,sr.second.soundpriority,voice);
      lastfreq=savefreq;
    }
    else {
      savefreq=lastfreq;
      if (soundflag)
        ::tone(firingplanesound,sr.second.soundpriority,voice);
      lastfreq=firingplanesound;
    }
  }
};

std::map<Object*,Soundpriority> soundrequests;
std::map<Object*,Sound*> sounds;

struct Higherpriority {
  inline bool operator()(const Soundrequest& lhs,const Soundrequest& rhs) const
  {
    return lhs.second<rhs.second;
  }
};

struct Lowerpriority {
  inline bool operator()(const std::pair<Object*,Sound*>& lhs,const std::pair<Object*,Sound*>& rhs) const
  {
    return lhs.second->sr.second>rhs.second->sr.second;
  }
};

void setsound(int newsoundpriority,int newsoundsubpriority,Object* newobject)
{
  if (soundrequests.find(newobject)!=soundrequests.end())
    soundrequests.insert(Soundrequest(newobject,Soundpriority(newsoundpriority,newsoundsubpriority,soundrequests.size()-1)));
  else
    soundrequests.insert(Soundrequest(newobject,Soundpriority(newsoundpriority,newsoundsubpriority,soundrequests.size())));
}

void killsounds()
{
  const std::map<Object*,Sound*>::const_iterator end=sounds.end();
  for (std::map<Object*,Sound*>::iterator sound=sounds.begin();sound!=end;) {
    std::map<Object*,Sound*>::iterator temp=sound;
    ++temp;
    delete sound->second;
    sounds.erase(sound);
    sound=temp;
  }
  soundoff();
}

void clearsounds()
{
  soundrequests.clear();
  titlemode=NO_MUSIC;
  killsounds();
}

bool removeleastprioritysound(const Soundpriority& currentsoundpriority)
{
  if (sounds.empty())
    return false;
  const std::set<std::pair<Object*,Sound*>,Lowerpriority> ascendingsounds(sounds.begin(),sounds.end());
  const std::set<std::pair<Object*,Sound*>,Lowerpriority>::const_iterator leastprioritysound=ascendingsounds.begin();
  if (currentsoundpriority<leastprioritysound->second->sr.second) {
    const std::map<Object*,Sound*>::iterator iter=sounds.find(leastprioritysound->first);
    delete iter->second;
    sounds.erase(iter);
    return true;
  }
  return false;
}

template<class T>
bool tone(unsigned short freq,const Soundrequest& sr)
{
  const std::map<Object*,Sound*>::iterator existingsound=sounds.find(sr.first);
  if (existingsound!=sounds.end()) {
    Sound* sound=existingsound->second;
    if (typeid(*sound)==typeid(T)) {
      while (!sound->tone(freq)) {
        if (!removeleastprioritysound(sr.second)) {
          delete sound;
          sounds.erase(existingsound);
          return false;
        }
      }
      return true;
    }
    else
      delete sound;
  }
  Sound* sound=new T(sr);
  while (!sound->tone(freq)) {
    if (!removeleastprioritysound(sr.second)) {
      delete sound;
      return false;
    }
  }
  sounds[sr.first]=sound;
  return true;
}

void updatesound()
{
  const std::list<Object*>::const_iterator end1=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end1;++obj) {
    if ((*obj)->hastone)
      (*obj)->tonefreq+=(*obj)->tonechange*soundticks;
  }

  soundticks=0;
  titlemode=NO_MUSIC;
  const std::map<Object*,Soundpriority>::const_iterator soundrequests_end=soundrequests.end();
  const std::map<Object*,Sound*>::const_iterator end2=sounds.end();
  for (std::map<Object*,Sound*>::iterator sound=sounds.begin();sound!=end2;) {
    const std::map<Object*,Soundpriority>::const_iterator existingsoundrequest=soundrequests.find(sound->first);
    if (existingsoundrequest==soundrequests_end) {
      std::map<Object*,Sound*>::iterator temp=sound;
      ++temp;
      delete sound->second;
      sounds.erase(sound);
      sound=temp;
    }
    else {
      sound->second->sr.second=existingsoundrequest->second;
      ++sound;
    }
  }
  bool room=true;
  const std::set<Soundrequest,Higherpriority> descendingsoundrequests(soundrequests.begin(),soundrequests.end());
  const std::set<Soundrequest,Higherpriority>::const_iterator end3=descendingsoundrequests.end();
  for (std::set<Soundrequest,Higherpriority>::const_iterator sr=descendingsoundrequests.begin();room && sr!=end3;++sr) {
    switch(sr->second.soundpriority) {
      case SOUND_PRIORITY_PLANE_OK:
        if (sr->second.soundsubpriority==0)
          room=tone<Sound>(stationaryplanesound,*sr);
        else
          room=tone<Sound>(sr->second.soundsubpriority*planespeedsoundmultiplier+movingplanebasesound,*sr);
        break;
      case SOUND_PRIORITY_BOMB_FALLING:
      case SOUND_PRIORITY_PLANE_FALLING:
        room=tone<Sound_slide>((sr->first)->tonefreq,*sr);
        break;
      case SOUND_PRIORITY_PLANE_TROUBLED:
        /* Use simulation RNG state instead of libc rand() to keep replay-validation runs deterministic. */
        room=tone<Sound>(((randv&1)!=0) ? troubledplanesound[0] : troubledplanesound[1],*sr);
        break;
      case SOUND_PRIORITY_EXPLOSION:
        room=tone<Sound>(explosion.tone,*sr);
        break;
      case SOUND_PRIORITY_FIRING:
        room=tone<Sound_shot>(firingsound,*sr);
        break;
      case SOUND_PRIORITY_THEME:
        if (soundflag && playtheme())
          titlemode=PLAY_SONG;
        else {
          title.place=0;
          titlenote();
          titlemode=PLAY_NOTE;
        }
      /*default:*/
    }
  }
  finalizesound();
  soundrequests.clear();
}

void soundadjust()
{
  ++soundticks;

  const std::map<Object*,Sound*>::const_iterator end=sounds.end();
  for (std::map<Object*,Sound*>::const_iterator sound=sounds.begin();sound!=end;++sound) {
    if ((sound->second)->lastfreq!=0)
      (sound->second)->soundadj();
  }

  if (numfrags!=0) {
    if (--explosion.ticks<0)
      explnote();
  }
  if (titlemode==PLAY_NOTE) {
    if (--title.ticks<0)
      titlenote();
  }
}

void titlenote()
{
  title.playnote();
  killsounds();
  tone<Sound>(title.tone,Soundrequest(0,Soundpriority(SOUND_PRIORITY_THEME,0,0)));
}

void explnote()
{
  explosion.playnote();
}

bool requestsoundevent(Soundevent soundevent,int soundpriority,int soundsubpriority)
{
  void* eventsample;
  if (!soundflag || (eventsample=sample(soundevent))==0)
    return false;
  Soundpriority soundpriority_(soundpriority,soundsubpriority,soundrequests.size());
  while (!playsoundevent(eventsample))
    if (!removeleastprioritysound(soundpriority_))
      return false;
  return true;
}

#endif /* SOPWITH_SOUNDSYS_CPP */
