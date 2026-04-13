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
#ifndef SOPWITH_SDL_JOYSTICK_H
#define SOPWITH_SDL_JOYSTICK_H

#include <SDL/SDL.h>
#include "event.h"

int joykeysprev;
int joykeyspressed;
int joykeysnext;

SDL_Joystick* joystick;
int x=0,y=0;
int buttons=0;

void initjoystick()
{
  std::cout<<"Initializing joystick...";

  SDL_JoystickEventState(SDL_ENABLE);
  joystick=SDL_JoystickOpen(0);

  std::cout<<"Done.\n";
}

void deinitjoystick()
{
  SDL_JoystickClose(joystick);
}

void updatejoy()
{
  static int joykeys[9]={KEY_DESCEND,KEY_DESCEND,KEY_DESCEND,
                         KEY_FLIP   ,0          ,KEY_FLIP,
                         KEY_CLIMB  ,KEY_CLIMB  ,KEY_CLIMB};
  pollEvents();
  if ((controls&JOYSTICK)!=0) {
    for (std::list<SDL_Event>::iterator event=events.begin();event!=events_end;) {
      bool erase=true;
      switch(event->type) {
        case SDL_JOYAXISMOTION:
          if (event->jaxis.axis==0) {
            if (event->jaxis.value<-3200)
              x=-1;
            else if (event->jaxis.value>3200)
              x=1;
            else
              x=0;
          }
          if (event->jaxis.axis==1) {
            if (event->jaxis.value<-3200)
              y=-1;
            else if (event->jaxis.value>3200)
              y=1;
            else
              y=0;
          }
          break;
        case SDL_JOYBUTTONDOWN:
          if (event->jbutton.button==0) {
            buttons|=KEY_FIRE;
          }
          if (event->jbutton.button==1) {
            buttons|=KEY_BOMB;
          }
          if (event->jbutton.button==2) {
            buttons|=KEY_BRAKE;
          }
          if (event->jbutton.button==3) {
            buttons|=KEY_ACCEL;
          }
          break;
        case SDL_JOYBUTTONUP:
          if (event->jbutton.button==0) {
            buttons&=~KEY_FIRE;
          }
          if (event->jbutton.button==1) {
            buttons&=~KEY_BOMB;
          }
          if (event->jbutton.button==2) {
            buttons&=~KEY_BRAKE;
          }
          if (event->jbutton.button==3) {
            buttons&=~KEY_ACCEL;
          }
          break;
        default:
          erase=false;
          break;
      }
      if (erase)
        event=events.erase(event);
      else
        ++event;
    }
    int j=joykeys[(x<0 ? 0 : (x>0 ? 2 : 1))+
                  (y<0 ? 0 : (y>0 ? 6 : 3))] | buttons;
    joykeyspressed=j;
    joykeysnext|=j;
    joykeysnext&=j|~joykeysprev;
  }
}

#endif /* SOPWITH_SDL_JOYSTICK_H */
