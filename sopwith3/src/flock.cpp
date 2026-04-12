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
#ifndef SOPWITH_FLOCK_CPP
#define SOPWITH_FLOCK_CPP

#include "flock.h"
#include "bird.h"
#include "sopwith.h"
#include "coll.h"
#include "graphics.h"
#include "smoke.h"

Flock::Flock(int x,int y,int xv)
{
  Flock::x=x;
  Flock::y=y;
  Flock::xv=xv;
  yv=0;
  inverted=false;
  life=FLOCKLIFE;
  width=FLOCK_WIDTH;
  height=FLOCK_HEIGHT;
  colour=colour_tan;
  add(childlist,new Bird(this,1));
}

bool Flock::update()
{
  if (life==-1) {
    return false;
  }
  if (--life==0) {
    inverted=!inverted;
    life=FLOCKLIFE;
  }
  if (x.integer<MIN_FLOCK_X || x.integer>MAX_FLOCK_X)
    xv=-xv.integer;
  move();
  sprite=spritedata(FLOCK,inverted ? 1 : 0);
  coll=flockspritescoll[inverted ? 1 : 0];
  return true;
}

bool Flock::onmap()
{
  return true;
}

void Flock::docollision(const Object* obj)
{
  if (!instanceof<Bird>(*obj) && !instanceof<Flock>(*obj) && !instanceof<Smoke>(*obj)) {
    for (int i=0;i<8;++i)
      add(childlist,new Bird(this,i));
    life=-1;
  }
}

#endif /* SOPWITH_FLOCK_CPP */
