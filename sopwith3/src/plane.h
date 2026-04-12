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
#ifndef SOPWITH_PLANE_H
#define SOPWITH_PLANE_H

#include "def.h"
#include "object.h"

class Plane : public Object {
public:
  enum Endstatus {NOTFINISHED,WINNER,LOSER};
  enum State {FLYING,CRASHED,FALLING,STALLED,WOUNDED,WOUNDSTALL};

  Plane(int runwayx,int colour,bool runwayleft,bool infinitefuel,bool infiniteammo,bool infinitebombs);
  virtual void init();
  virtual void reinit();
  virtual bool update();
  virtual void processkeys(int keys);
  virtual bool onmap();
  virtual void docollision(const Object* obj);
  virtual void sound();
  void fill();
  void gohome();
  void aim(int destx,int desty,Object* target,bool longway,int courseadjust);
  bool doomed(int x,int y,int alt);
  void stall();
  void hit();
  virtual void refuel();
  bool topup(int& val,int max);
  void crash();
  virtual void burnfuel();
  int gunontarget(Object* target,Object* tempplane);
  bool flying() const {return state==FLYING || state==WOUNDED;}
  bool wounded() const {return state==WOUNDED || state==WOUNDSTALL;}
  bool stalled() const {return state==STALLED || state==WOUNDSTALL;}
  bool active() const {return state==FLYING || state==STALLED || state==WOUNDED || state==WOUNDSTALL;}
public:
  Endstatus endstatus;
  State state;
  bool collided;
  int lives,maxlives;
  short score;
  int ammo;
  int bombs;
  bool athome;
  int endcounter;
protected:
  int runwayx;
  int runwayy;
  bool runwayleft;
  int collxv;
  int collyv;
  int accel,flaps;
  bool bombing;
  int bomb_delay;
  bool goinghome;
  bool infinitefuel,infiniteammo,infinitebombs;
};

#endif /* SOPWITH_PLANE_H */
