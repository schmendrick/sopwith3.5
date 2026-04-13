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
#ifndef SOPWITH_SDL_KEYBOARD_H
#define SOPWITH_SDL_KEYBOARD_H

#include <SDL/SDL.h>
#include "event.h"

volatile bool breakf;
int keysprev;
volatile int keysnext;
volatile int keyspressed;

int inkey()
{
  pollEvents();
  for (std::list<SDL_Event>::iterator event=events.begin();event!=events_end;++event) {
    switch(event->type) {
      case SDL_KEYDOWN:
        if (event->key.keysym.sym>=1 && event->key.keysym.sym<=0xff) {
          int sym=event->key.keysym.sym;
          events.erase(event);
          return sym;
        }
        break;
      default:
        break;
    }
  }
  return 0;
}

void flushkeybuf()
{
  pollEvents();
  for (std::list<SDL_Event>::iterator event=events.begin();event!=events_end;) {
    bool erase=true;
    switch(event->type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
      default:
        erase=false;
        break;
    }
    if (erase)
      event=events.erase(event);
    else
      ++event;
  }
}

void pollkeyboard()
{
  if (!ibmkeyboard)
    return;
  pollEvents();
  for (std::list<SDL_Event>::iterator event=events.begin();event!=events_end;) {
    bool erase=true;
    switch(event->type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        {
          int keymask=0;
          switch (event->key.keysym.sym) {
            case SDLK_x:         keymask=KEY_ACCEL;     break;
            case SDLK_z:         keymask=KEY_BRAKE;     break;
            case SDLK_COMMA:     keymask=KEY_CLIMB;     break;
            case SDLK_SLASH:     keymask=KEY_DESCEND;   break;
            case SDLK_PERIOD:    keymask=KEY_FLIP;      break;
            case SDLK_SPACE:     keymask=KEY_FIRE;      break;
            case SDLK_b:         keymask=KEY_BOMB;      break;
            case SDLK_h:         keymask=KEY_GOHOME;    break;
            case SDLK_s:         keymask=KEY_SOUND;     break;
            case SDLK_ESCAPE:
            case SDLK_SCROLLOCK: keymask=KEY_BREAK;     breakf=true; break;
            case SDLK_p:
              keymask=KEY_PAUSEGAME;
              break;
            default: keymask=0;
          }
          if (keymask!=0) {
            if (event->type==SDL_KEYUP) {
              if ((keymask&keysprev)!=0)
                keysnext&=~keymask;
              keyspressed&=~keymask;
            }
            else {
              keyspressed|=keymask;
              keysnext|=keymask;
            }
          }
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
}

void initkeyboard()
{
}

void deinitkeyboard()
{
}

#endif /* SOPWITH_SDL_KEYBOARD_H */
