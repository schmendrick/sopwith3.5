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
#ifndef SOPWITH_FRAG_CPP
#define SOPWITH_FRAG_CPP

#include "frag.h"
#include "coll.h"
#include "sopwith.h"
#include "graphics.h"
#include "soundsys.h"
#include "bird.h"
#include "flock.h"
#include "smoke.h"

Frag::Frag(S_fix x,S_fix y,S_fix xv,S_fix yv,int type,int colour,bool noisy)
{
  Frag::x=x;
  Frag::y=y;
  Frag::xv=xv;
  Frag::yv=yv;
  Frag::type=type;
  Frag::colour=colour;
  life=FRAGLIFE;
  hitcounter=speed=0;
  width=FRAG_WIDTH;
  height=FRAG_HEIGHT;
  Frag::noisy=noisy;
  if (noisy)
    initsound(SOUND_PRIORITY_EXPLOSION);
}

bool Frag::update()
{
  #ifdef DEBUG
  errorfile<<'('<<type<<')';
  #endif
  if (life<0) {
    if (noisy)
      stopsound();
    return false;
  }
  if (--life==0) {
    if (yv.integer<0) {
      if (xv.integer<0)
        ++xv;
      else {
        if (xv.integer>0)
          --xv;
      }
    }
    if (yv.integer>(type==FRAG_PERSON ? -minspeed : TERMINAL_VELOCITY))
      --yv;
    life=FRAGLIFE;
  }
  move();
  if (y.integer<=static_cast<int>(ground[x.integer]) || x.integer<0 || x.integer>=MAX_X) {
    if (noisy)
      stopsound();
    return false;
  }
  ++hitcounter;
  sprite=spritedata(FRAG,colour==colour_cyan ? 1 : 2,type);
  coll=fragspritescoll[colour==colour_cyan ? 1 : 2][type];
  return true;
}

void Frag::sound()
{
  if (noisy)
    setsound(SOUND_PRIORITY_EXPLOSION,hitcounter,0);
}

void Frag::initsound(int /*soundpriority*/)
{
  if (!hastone) {
    if (++numfrags==1) {
      explosion.place=0;
      explnote();
    }
    hastone=true;
  }
}

void Frag::stopsound()
{
  if (hastone) {
    --numfrags;
    hastone=false;
  }
}

void Frag::docollision(const Object* obj)
{
  if (!instanceof<Bird>(*obj) && !instanceof<Flock>(*obj) &&
      !instanceof<Frag>(*obj) && !instanceof<Smoke>(*obj) &&
      (obj==0 || (version==2 && (type!=FRAG_PERSON ||
       (type==FRAG_PERSON && scorepenalty(obj,200)))))) {
    life=1;
    stopsound();
  }
}

#endif /* SOPWITH_FRAG_CPP */
