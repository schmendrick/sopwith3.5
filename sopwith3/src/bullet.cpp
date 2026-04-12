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
#ifndef SOPWITH_BULLET_CPP
#define SOPWITH_BULLET_CPP

#include "bullet.h"
#include "sopwith.h"
#include "coll.h"
#include "graphics.h"
#include "bird.h"
#include "flock.h"
#include "smoke.h"

Bullet::Bullet(Object* obj,S_fix xv,S_fix yv)
{
  Bullet::xv=xv;
  Bullet::yv=yv;
  x=obj->x+PLANE_WIDTH/2;
  y=obj->y-PLANE_HEIGHT/2;
  life=BULLETLIFE;
  colour=obj->colour;
  width=BULLET_WIDTH;
  height=BULLET_HEIGHT;
  speed=0;
}

bool Bullet::update()
{
  if (--life==0)
    return false;
  move();
  if (x.integer<0 || x.integer>=MAX_X || y.integer<=static_cast<int>(ground[x.integer]) /*|| y>=MAX_Y*/)
    return false;
  sprite=spritedata(PIXEL,0);
  coll=pixelspritescoll;
  return true;
}

void Bullet::docollision(const Object* obj)
{
  if (!instanceof<Bird>(*obj) && !instanceof<Flock>(*obj) && !instanceof<Smoke>(*obj))
    life=1;
}

#endif /* SOPWITH_BULLET_CPP */
