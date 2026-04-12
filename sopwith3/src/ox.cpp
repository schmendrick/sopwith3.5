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
#ifndef SOPWITH_OX_CPP
#define SOPWITH_OX_CPP

#include "ox.h"
#include "graphics.h"
#include "coll.h"
#include "bird.h"
#include "flock.h"
#include "smoke.h"
#include "frag.h"

Ox::Ox(int x,int y) :
  state(ALIVE)
{
  Ox::x=x;
  Ox::y=y;
  xv=0;
  yv=0;
  height=OX_WIDTH;
  width=OX_HEIGHT;
  colour=colour_cyan;
}

bool Ox::update()
{
  sprite=spritedata(OX,state==ALIVE ? 0 : 1);
  coll=oxspritescoll[state==ALIVE ? 0 : 1];
  return true;
}

void Ox::docollision(const Object* obj)
{
  if (instanceof<Bird>(*obj) || instanceof<Flock>(*obj) || instanceof<Smoke>(*obj))
    return;
  if (state!=ALIVE)
    return;
  if (instanceof<Frag>(*obj))
    return;
  scorepenalty(obj,200);
  state=DEAD;
}

#endif /* SOPWITH_OX_CPP */
