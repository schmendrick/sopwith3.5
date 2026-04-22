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
#ifndef SOPWITH_BOMB_CPP
#define SOPWITH_BOMB_CPP

#include "bomb.h"
#include "sopwith.h"
#include "coll.h"
#include "graphics.h"
#include "sound.h"
#include "soundsys.h"
#include "bird.h"
#include "flock.h"
#include "smoke.h"

Bomb::Bomb(Plane* plane)
{
  xv=plane->xv.integer;
  yv=plane->yv.integer;
  int drop_angle;
  if (plane->inverted)
    drop_angle=(plane->angle+ANGLES/4)&(ANGLES-1);
  else
    drop_angle=(plane->angle-ANGLES/4)&(ANGLES-1);
  angle=drop_angle;
  x=plane->x.integer+s_cos(10,drop_angle).integer+4;
  y=plane->y.integer+s_sin(10,drop_angle).integer-4;
  life=BOMBLIFE;
  colour=plane->colour;
  width=BOMB_WIDTH;
  height=BOMB_HEIGHT;
}

bool Bomb::update()
{
  if (life<0)
    return false;
  adjustfall();
  if (yv.integer<=0)
    initsound(SOUND_PRIORITY_BOMB_FALLING);
  move();
  if (x.integer<0 || x.integer>=MAX_X || y.integer<0) {
    stopsound();
    return false;
  }
  /* http://gcc.gnu.org/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail&database=gcc&pr=7209 */
  int dir=direction();
  angle=dir;
  sprite=spritedata(BOMB,colour==colour_cyan ? 0 : 1,dir);
  coll=bombspritescoll[dir];
  return true;
}

bool Bomb::onmap()
{
  return true;
}

void Bomb::docollision(const Object* obj)
{
  if (!instanceof<Bird>(*obj) && !instanceof<Flock>(*obj) && !instanceof<Smoke>(*obj)) {
    /* Explosion branch randomness intentionally uses randv to stay on the deterministic replay stream. */
    createexplosion(minspeed>>(version==2||(randv&7)!=7 ? 1 : 0),2,false,!(onscreen(this) && requestsoundevent(BOMB_DESTROYED,SOUND_PRIORITY_EXPLOSION,0)));
    life=-1;
    if (obj==0)
      crater();
    stopsound();
  }
}

void Bomb::sound()
{
  if (yv.integer<=0)
    setsound(SOUND_PRIORITY_BOMB_FALLING,-y.integer,this);
}

void Bomb::adjustfall()
{
  if (--life==0) {
    if (yv.integer<0) {
      if (xv.integer<0)
        ++xv;
      else {
        if (xv.integer>0)
          --xv;
      }
    }
    if (yv.integer>TERMINAL_VELOCITY)
      --yv;
    life=BOMBLIFE;
  }
}

#endif /* SOPWITH_BOMB_CPP */
