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
#ifndef SOPWITH_ALLEGRO_JOYSTICK_H
#define SOPWITH_ALLEGRO_JOYSTICK_H

#include <allegro.h>
#include "../graphics.h"
#include "../keyboard.h"

int joykeysprev;
int joykeyspressed;
int joykeysnext;

void initjoystick()
{
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Center joystick and press any key to continue");
  displayscreen();
  while (!inkey())
    processtimerticks();
  install_joystick(JOY_TYPE_AUTODETECT);
}

void deinitjoystick()
{
  remove_joystick();
}

void updatejoy()
{
  poll_joystick();
  static int joykeys[9]={KEY_DESCEND,KEY_DESCEND,KEY_DESCEND,
                         KEY_FLIP   ,0          ,KEY_FLIP,
                         KEY_CLIMB  ,KEY_CLIMB  ,KEY_CLIMB};

  if ((controls&JOYSTICK)!=0) {
    int j=joykeys[(joy[0].stick[0].axis[0].d1!=0 ? 0 : (joy[0].stick[0].axis[0].d2!=0 ? 2 : 1))+
                  (joy[0].stick[0].axis[1].d1!=0 ? 0 : (joy[0].stick[0].axis[1].d2!=0 ? 6 : 3))];
    if (joy[0].button[0].b!=0)
      j|=KEY_FIRE;
    if (joy[0].button[1].b!=0)
      j|=KEY_BOMB;
    if (joy[0].button[2].b!=0)
      j|=KEY_BRAKE;
    if (joy[0].button[3].b!=0)
      j|=KEY_ACCEL;
    joykeyspressed=j;
    joykeysnext|=j;
    joykeysnext&=j|~joykeysprev;
  }
}

#endif /* SOPWITH_ALLEGRO_JOYSTICK_H */
