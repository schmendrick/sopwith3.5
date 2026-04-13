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
#ifndef SOPWITH_SDL_TIMER_H
#define SOPWITH_SDL_TIMER_H

#include <SDL/SDL.h>

namespace {
  Uint32 ticksStart;
}

void inittimer()
{
  std::cout<<"Initializing timer...";

  ticksStart=SDL_GetTicks();

  std::cout<<"Done.\n";
}

unsigned long timer()
{
  static float ticksPerMilliseconds=clockTickRate/(clockTicksPerTicks*1000.0f);
  return static_cast<unsigned long>((SDL_GetTicks()-ticksStart)*ticksPerMilliseconds);
}

void timeridle()
{
  SDL_Delay(1);
}

void deinittimer()
{
}

#endif /* SOPWITH_SDL_TIMER_H */
