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
#ifndef SOPWITH_OBJECT_H
#define SOPWITH_OBJECT_H

#include "def.h"
#include "s_fix.h"
#include <list>

class Object {
public:
  Object();
  virtual ~Object() {}
  virtual bool update()=0;
  virtual bool onmap();
  virtual void docollision(const Object* /*obj*/) {}
  virtual void sound() {}
  void initsound(int soundpriority);
  void stopsound();
  void setvel(int v,int dir);
  int direction();
  void createexplosion(int speed,int step,bool pilotflag,bool noisy=true);
  void crater();
  void move();
  std::list<Object*> childlist;
/**/
  S_fix x,y,xv,yv;
  int angle;
  bool inverted;
  int speed;
  bool firing;
  int hitcounter;
  int life;
  int width,height;
  int colour;
  void* sprite;
  unsigned char* coll;
/**/
  bool hastone;
  unsigned short tonefreq;
  int tonechange;
};

int range(int x0,int y0,int x1,int y1);
void scoretarget(const Object* destroyed,int score);
bool scorepenalty(const Object* obj,int score);

#endif /* SOPWITH_OBJECT_H */
