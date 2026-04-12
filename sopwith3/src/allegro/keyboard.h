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
#ifndef SOPWITH_ALLEGRO_KEYBOARD_H
#define SOPWITH_ALLEGRO_KEYBOARD_H

#include <allegro.h>

volatile bool breakf;
int keysprev;
volatile int keysnext;
volatile int keyspressed;

int inkey()
{
  if (keypressed())
    return readkey()&0xff;
  return 0;
}

void flushkeybuf()
{
  while (keypressed())
    readkey();
}

void pollkeyboard()
{
}

namespace {
  void keybint(int scancode)
  {
    int k;
    if (!ibmkeyboard)
      return;
    switch (scancode&0x7f) {
      case KEY_X:       k=KEY_ACCEL;     break;
      case KEY_Z:       k=KEY_BRAKE;     break;
      case KEY_COMMA:   k=KEY_CLIMB;     break;
      case KEY_SLASH:   k=KEY_DESCEND;   break;
      case KEY_STOP:    k=KEY_FLIP;      break;
      case KEY_SPACE:   k=KEY_FIRE;      break;
      case KEY_B:       k=KEY_BOMB;      break;
      case KEY_H:       k=KEY_GOHOME;    break;
      case KEY_S:       k=KEY_SOUND;     break;
      case KEY_SCRLOCK: k=KEY_BREAK;     breakf=true; break;
      case KEY_P:
        k=KEY_PAUSEGAME;
        break;
      default: k=0;
    }
    if (k!=0)
      if ((scancode&0x80)!=0) {
        if ((k&keysprev)!=0)
          keysnext&=~k;
        keyspressed&=~k;
      }
      else {
        keyspressed|=k;
        keysnext|=k;
      }
  }
  END_OF_FUNCTION(keybint)
}

void initkeyboard()
{
  LOCK_FUNCTION(keybint);
  LOCK_VARIABLE(ibmkeyboard);
  LOCK_VARIABLE(breakf);
  LOCK_VARIABLE(keysprev);
  LOCK_VARIABLE(keysnext);
  LOCK_VARIABLE(keyspressed);

  install_keyboard();
  keyboard_lowlevel_callback=keybint;
}

void deinitkeyboard()
{
  remove_keyboard();
}

#endif /* SOPWITH_ALLEGRO_KEYBOARD_H */
