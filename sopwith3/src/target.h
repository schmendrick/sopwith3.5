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
#ifndef SOPWITH_TARGET_H
#define SOPWITH_TARGET_H

#include "def.h"
#include "object.h"

class Target : public Object {
public:
  enum Type {FLAG,CHIMNEY,FUEL,TANK};
  enum State {INTACT,DESTROYED};

  Target(int x,int colour,Type type);
  virtual bool update();
  virtual bool onmap();
  virtual void docollision(const Object* obj);
  virtual void sound();
public:
  Type type;
  State state;
private:
  bool noisy;
};

int range2(int x0,int y0,int x1,int y1);

#endif /* SOPWITH_TARGET_H */
