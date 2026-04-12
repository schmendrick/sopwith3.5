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
#ifndef SOPWITH_COMPUTER_CPP
#define SOPWITH_COMPUTER_CPP

#include "computer.h"
#include "sopwith.h"
#include "target.h"
#include <algorithm>

Computerplane::Computerplane(int runwayx,int colour,bool runwayleft,bool unlimitedterr,int left_terr,int right_terr,bool infinitefuel,bool infiniteammo,bool infinitebombs) :
  Plane(runwayx,colour,runwayleft,infinitefuel,infiniteammo,infinitebombs),
  unlimitedterr(unlimitedterr),
  left_terr(left_terr),
  right_terr(right_terr),
  nearobject(0) {}

bool Computerplane::update()
{
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    Plane* plane=dynamic_cast<Plane*>(*obj);
    if (plane!=0 && plane->active() && plane->endstatus==NOTFINISHED)
      alert(plane);
    else
      if (gamemode==MULTIPLE) {
        Target* target=dynamic_cast<Target*>(*obj);
        if (target!=0 && target->state==Target::INTACT)
          alert(target);
      }
  }
  flaps=0;
  bombing=false;
  if (latencycount==0)
    firing=false;
  switch (state) {
    case WOUNDED:
    case WOUNDSTALL:
      if ((framecounter&1)==1)
        break;
    case FLYING:
    case STALLED:
      if (endstatus!=NOTFINISHED)
        gohome();
      else
        if (latencycount==0)
          pilot();
      break;
    case CRASHED:
      firing=false;
      if (hitcounter<=0 && endstatus==NOTFINISHED)
        reinit();
      break;
    default:
      firing=false;
  }
  return Plane::update();
}

void Computerplane::pilot()
{
  if (nearobject!=0)
    attack(nearobject);
  else
    if (!athome)
      cruise();
  nearobject=0;
}

void Computerplane::alert(Object* obj)
{
  if (colour!=obj->colour && (unlimitedterr || (left_terr<=obj->x.integer && obj->x.integer<=right_terr)) &&
      (nearobject==0 || std::abs(obj->x.integer-x.integer)<std::abs(nearobject->x.integer-x.integer)))
    nearobject=obj;
}

void Computerplane::attack(Object* target)
{
  if (target->speed!=0)
    aim(target->x.integer-s_cos(CLOSE,target->angle).integer,
        target->y.integer-s_sin(CLOSE,target->angle).integer,target,false,((framecounter&0x1f)<16 ? 16 : 0));
  else
    aim(target->x.integer,target->y.integer+4,target,false,((framecounter&0x1f)<16 ? 16 : 0));
}

void Computerplane::cruise()
{
  int courseadjust=((framecounter&0x1f)<16 ? 16 : 0);
  aim(std::max(MAX_X/3,std::min(runwayx,2*MAX_X/3))+courseadjust,
      CRUISEHEIGHT-(courseadjust>>1),0,false,courseadjust);
}

#endif /* SOPWITH_COMPUTER_CPP */
