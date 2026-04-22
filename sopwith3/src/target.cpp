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
#ifndef SOPWITH_TARGET_CPP
#define SOPWITH_TARGET_CPP

#include "target.h"
#include "sopwith.h"
#include "sound.h"
#include "soundsys.h"
#include "coll.h"
#include "bullet.h"
#include "bird.h"
#include "flock.h"
#include "computer.h"
#include "graphics.h"
#include "smoke.h"
#include "frag.h"

Target::Target(int x,int colour,Type type) :
  type(type),
  state(INTACT),
  noisy(true)
{
  Target::x=x;
  int minaltitude=INT_MAX;
  int maxaltitude=0;
  int maxx=x+15;
  for (x=Target::x.integer;x<=maxx;x++) {
    maxaltitude=std::max(ground[x],maxaltitude);
    minaltitude=std::min(ground[x],minaltitude);
  }
  y=std::min((minaltitude+maxaltitude)>>1,MAX_Y-17);
  for (x=Target::x.integer;x<=maxx;x++)
    ground[x]=y.integer;
  y.integer+=16;
  xv=0;
  yv=0;
  hitcounter=0;
  Target::colour=colour;
  width=TARGET_WIDTH;
  height=TARGET_HEIGHT;
}

bool Target::update()
{
  firing=false;
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    Plane* plane=dynamic_cast<Plane*>(*obj);
    if (plane!=0 && (gamemode==MULTIPLE || !instanceof<Computerplane>(*plane)))
      if (level!=0 && state==INTACT && plane->active() &&
          colour!=plane->colour && (level>1 || (framecounter&1)==1)) {
        int d2=range(x.integer,y.integer,plane->x.integer,plane->y.integer);
        if (d2>0 && d2<targetrange2) {
          firing=true;
          int bspeed=level+BULLETSPEED;
          int x=plane->x.integer+(plane->xv.integer<<2);
          int y=plane->y.integer+(plane->yv.integer<<2);
          int xv=x-Target::x.integer;
          int yv=y-Target::y.integer;
          int r=range2(x,y,Target::x.integer,Target::y.integer);
          if (r>=1) {
            add(childlist,new Bullet(this,S_fix((xv*bspeed)/r),S_fix((yv*bspeed)/r)));
            noisy=!(onscreen(this) && requestsoundevent(TARGET_FIRED,SOUND_PRIORITY_FIRING,0));
          }
          break;
        }
      }
  }
  if (--hitcounter<0)
    hitcounter=0;
  if (state==INTACT) {
    sprite=spritedata(TARGET,colour==colour_cyan ? 0 : 1,type,(framecounter/2)&3);
    coll=targetspritescoll[type];
  }
  else {
    sprite=spritedata(DEBRIS,colour==colour_cyan ? 0 : 1);
    coll=debrisspritescoll;
  }
  return true;
}

bool Target::onmap()
{
  return state==INTACT;
}

void Target::docollision(const Object* obj)
{
  if (instanceof<Bird>(*obj) || instanceof<Flock>(*obj) || instanceof<Smoke>(*obj))
    return;
  if (state!=INTACT)
    return;
  if (instanceof<Frag>(*obj))
    return;
  if (instanceof<Bullet>(*obj)) {
    hitcounter+=TARGETHITCOUNT;
    if (hitcounter<TARGETHITCOUNT*(level+1) || (version!=2 && hitcounter==TARGETHITCOUNT*(level+1)))
      return;
  }
  state=DESTROYED;
  if (type==FUEL)
    createexplosion((version==2 ? maxspeed : minspeed),1,false,!(onscreen(this) && requestsoundevent(TARGET_DESTROYED,SOUND_PRIORITY_EXPLOSION,0)));
  else
    /* Keep non-fuel target explosion randomness on randv stream for deterministic replays. */
    createexplosion(minspeed>>(version==2||(randv&7)!=7 ? 1 : 0),2,false,!(onscreen(this) && requestsoundevent(TARGET_DESTROYED,SOUND_PRIORITY_EXPLOSION,0)));
  scoretarget(this,this->type==FUEL ? 200 : 100);
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj2=objectlist.begin();obj2!=end;++obj2) {
    Target* target;
    if ((target=dynamic_cast<Target*>(*obj2))!=0 && colour==target->colour && target->state==INTACT)
      return;
  }
  endgame(colour);
}

void Target::sound()
{
  if (firing && noisy)
    setsound(SOUND_PRIORITY_FIRING,0,this);
}

int range2(int x0,int y0,int x1,int y1)
{
  int x=std::abs(x0-x1),y=std::abs(y0-y1),t;
  if (version!=2)
    y+=(y>>1);
  if (x>100 || y>100)
    return -1;
  if (x<y) { t=x; x=y; y=t; }
  return (x*7+(y<<2))>>3;
}

#endif /* SOPWITH_TARGET_CPP */
