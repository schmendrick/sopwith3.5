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
#ifndef SOPWITH_OBJECT_CPP
#define SOPWITH_OBJECT_CPP

#include "object.h"
#include "target.h"
#include "plane.h"
#include "frag.h"
#include "bullet.h"
#include "sopwith.h"
#include "ground.h"
#include "graphics.h"
#include "player.h"
#include "bomb.h"
#include <algorithm>

Object::Object() :
  speed(0),
  /**/sprite(0),/**/
  /**/coll(0),/**/
  hastone(false)
{}

bool Object::onmap()
{
  return false;
}

void Object::initsound(int soundpriority)
{
  if (!hastone) {
    hastone=true;
    switch (soundpriority) {
      case SOUND_PRIORITY_BOMB_FALLING:
        tonefreq=0x300;
        tonechange=8;
        break;
      case SOUND_PRIORITY_PLANE_FALLING:
        tonefreq=0x1200;
        tonechange=-8;
        break;
    }
  }
}

void Object::stopsound()
{
  hastone=false;
}

void Object::setvel(int v,int dir)
{
  xv=s_cos(v,dir);
  yv=s_sin(v,dir);
}

int Object::direction()
{
  if (yv.integer>0)  { if (xv.integer<0) return 3; if (xv.integer==0) return 2; return 1; }
  else if (yv.integer==0) { if (xv.integer<0) return 4; if (xv.integer==0) return 6; return 0; }
  else { if (xv.integer<0) return 5; if (xv.integer==0) return 6; return 7; }
}

void Object::createexplosion(int speed,int step,bool pilotflag,bool noisy)
{
  for (int i=1;i<=15;i+=step) {
    S_fix xv=s_cos(speed,i)+(Object::xv.integer>>(version==2 ? 0 : 2));
    S_fix yv=s_sin(speed,i)+(Object::yv.integer>>(version==2 ? 0 : 2));
    S_fix x(xv.integer+Object::x.integer+(width>>1));
    S_fix y(yv.integer+Object::y.integer+(height>>1));
    int type;
    if (version==2) {
      int randv1=y.integer*x.integer*randv*7491;
      type=static_cast<int>(
            (
             static_cast<unsigned long>(
              static_cast<unsigned short>(
               static_cast<unsigned long>(randv1)*static_cast<unsigned long>(i)
              )
             )*8UL
            )>>16
           );
    }
    else {
      randv=y.integer*x.integer*randv+7491;
      if (randv==0)
        randv=static_cast<short>(74917777L);
      type=(randv&0x1c0)>>6;
    }
    if (pilotflag && (type==Frag::FRAG_PERSON || type==Frag::FRAG_DUST)) {
      pilotflag=false;
      type=Frag::FRAG_PERSON;
      xv.integer=Object::xv.integer;
      yv.integer=-minspeed;
    }
    add(childlist,new Frag(x,y,xv,yv,type,colour,type!=Frag::FRAG_PERSON && noisy));
  }
}

void Object::crater()
{
  static int depth[8]={1,2,2,3,3,2,2,1};

  int left=x.integer+(width-8)/2;
  int right=left+7;
  for (int x=left,i=0;x<=right;++x,++i)
    ground[x]=std::max(ground[x]-depth[i],std::max(originalground[x]-20,20));
  forceredraw=true;
}

void Object::move()
{
  x+=xv;
  y+=yv;
}

int range(int x0,int y0,int x1,int y1)
{
  int x=std::abs(x0-x1),y=std::abs(y0-y1),t;
  if (version!=2)
    y+=(y>>1);
  if (x<125 && y<125)
    return x*x+y*y;
  if (x<y) { t=x; x=y; y=t; }
  return -((x*7+(y<<2))>>3);
}

void scoretarget(const Object* destroyed,int score)
{
  if (gamemode!=MULTIPLE) {
    if (destroyed->colour==colour_cyan)
      headplanes[0]->score-=score;
    else
      headplanes[0]->score+=score;
    displayscore(headplanes[0]);
  }
  else {
    headplanes[destroyed->colour==colour_cyan ? 1 : 0]->score+=score;
    displayscore(headplanes[destroyed->colour==colour_cyan ? 1 : 0]);
  }
}

bool scorepenalty(const Object* obj,int score)
{
  const Plane* plane=dynamic_cast<const Plane*>(obj);
  if ((plane!=0 && (plane->flying() ||
                   (plane->state==Plane::FALLING && plane->hitcounter==FALLCOUNT)) &&
       !plane->athome) || instanceof<Bullet>(*obj) || instanceof<Bomb>(*obj)) {
    scoretarget(obj,score);
    return true;
  }
  return false;
}

#endif /* SOPWITH_OBJECT_CPP */
