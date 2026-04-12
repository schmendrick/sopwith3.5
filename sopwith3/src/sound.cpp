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
#ifndef SOPWITH_SOUND_CPP
#define SOPWITH_SOUND_CPP

#include "sound.h"
#include "sopwith.h"

#ifdef SDL
#include "sdl/sound.h"
#elif defined(ALLEGRO)
#include "allegro/sound.h"
#elif defined(__DJGPP__)
#include "djgpp/sound.h"
#else
void initsound() {}
void deinitsound() {}
void soundoff() {}
void deletevoice(void* /*voice*/) {}
bool tone(unsigned short /*freq*/,int /*soundpriority*/,void*& /*voice*/)
{
  return false;
}
void finalizesound() {}
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
#endif

#endif /* SOPWITH_SOUND_CPP */
