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
#ifndef SOPWITH_BIRD_CPP
#define SOPWITH_BIRD_CPP

#include "bird.h"
#include "sopwith.h"
#include "coll.h"
#include "ground.h"
#include "graphics.h"
#include "flock.h"
#include "smoke.h"

Bird::Bird(Object* flock,int birdno)
{
  static int fscatterx[8]={8,3,0,6,7,14,10,12};
  static int fscattery[8]={16,1,8,3,12,10,7,14};
  static int fscatterxv[8]={-2,2,-3,3,-1,1,0,0};
  static int fscatteryv[8]={-1,-2,-1,-2,-1,-2,-1,-2};

  x=flock->x+fscatterx[birdno];
  y=flock->y-fscattery[birdno];
  xv=fscatterxv[birdno];
  yv=fscatteryv[birdno];
  inverted=false;
  life=BIRDLIFE;
  width=BIRD_WIDTH;
  height=BIRD_HEIGHT;
  colour=flock->colour;
}

bool Bird::update()
{
  if (life==-1) {
    return false;
  }
  if (life==-2) {
    yv=-yv.integer;
    xv=(framecounter&7)-4;
    life=BIRDLIFE;
  }
  else {
    if (--life==0) {
      inverted=!inverted;
      life=BIRDLIFE;
    }
  }
  move();
  if (x.integer<0 || x.integer>=MAX_X || y.integer<=static_cast<int>(ground[x.integer]) || y.integer>=MAX_Y) {
    y-=yv;
    life=-2;
    sprite=0;
  }
  else
    sprite=spritedata(BIRD,inverted ? 1 : 0);
  coll=birdspritescoll[inverted ? 1 : 0];
  return true;
}

void Bird::docollision(const Object* obj)
{
  if (!instanceof<Bird>(*obj) && !instanceof<Flock>(*obj) && !instanceof<Smoke>(*obj)) {
    life=(scorepenalty(obj,25) ? -1 : -2);
  }
}

#endif /* SOPWITH_BIRD_CPP */
