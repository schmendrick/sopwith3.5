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
#ifndef SOPWITH_JOYSTICK_CPP
#define SOPWITH_JOYSTICK_CPP

#include "joystick.h"
#include "sopwith.h"

#ifdef SDL
#include "sdl/joystick.h"
#elif defined(ALLEGRO)
#include "allegro/joystick.h"
#elif defined(__DJGPP__)
#include "djgpp/joystick.h"
#else
int joykeysprev=0;
int joykeyspressed=0;
int joykeysnext=0;

void initjoystick() {}
void deinitjoystick() {}
void updatejoy() {}
#endif

#endif /* SOPWITH_JOYSTICK_CPP */
