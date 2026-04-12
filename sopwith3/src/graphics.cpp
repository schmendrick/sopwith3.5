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
#ifndef SOPWITH_GRAPHICS_CPP
#define SOPWITH_GRAPHICS_CPP

#include "graphics.h"
#include "title.h"
#include "font.h"

#ifdef SDL
#include "sdl/graphics.h"
#elif defined(ALLEGRO)
#include "allegro/graphics.h"
#elif defined(__DJGPP__)
#include "djgpp/graphics.h"
#else
#error No graphics functionality implemented for current platform
#endif

#endif /* SOPWITH_GRAPHICS_CPP */
