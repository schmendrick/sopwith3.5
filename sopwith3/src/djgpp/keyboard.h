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
#ifndef SOPWITH_DJGPP_KEYBOARD_H
#define SOPWITH_DJGPP_KEYBOARD_H

#include "misc.h"
#include <pc.h>

volatile bool breakf;
int keysprev;
volatile int keysnext;
volatile int keyspressed;

int inkey()
{
  if (kbhit())
    return getkey();
  return 0;
}

void flushkeybuf()
{
  while (kbhit())
    getkey();
}

void pollkeyboard()
{
}

namespace {
  _go32_dpmi_seginfo oldkeyboard;

  enum {
    SC_P    =0x19,
    SC_S    =0x1f,
    SC_H    =0x23,
    SC_Z    =0x2c,
    SC_X    =0x2d,
    SC_B    =0x30,
    SC_COMMA=0x33,
    SC_DOT  =0x34,
    SC_SLASH=0x35,
    SC_SPACE=0x39,
    SC_BREAK=0x46
  };

  unsigned char inportb_locked(unsigned short _port)
  {
    unsigned char output;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (output) : "dN" (_port));
    return output;
  }
  END_OF_FUNCTION(inportb_locked)

  void keybint()
  {
    int scancode,k;
    if (!ibmkeyboard)
      return;
    scancode=inportb_locked(0x60);
    switch (scancode&0x7f) {
      case SC_X:     k=KEY_ACCEL;     break;
      case SC_Z:     k=KEY_BRAKE;     break;
      case SC_COMMA: k=KEY_CLIMB;     break;
      case SC_SLASH: k=KEY_DESCEND;   break;
      case SC_DOT:   k=KEY_FLIP;      break;
      case SC_SPACE: k=KEY_FIRE;      break;
      case SC_B:     k=KEY_BOMB;      break;
      case SC_H:     k=KEY_GOHOME;    break;
      case SC_S:     k=KEY_SOUND;     break;
      case SC_BREAK: k=KEY_BREAK;     breakf=true; break;
      case SC_P:
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
  _go32_dpmi_seginfo newkeyboard;

  newkeyboard.pm_selector=_go32_my_cs();
  newkeyboard.pm_offset=reinterpret_cast<int>(keybint);

  LOCK_FUNCTION(keybint);
  LOCK_VARIABLE(ibmkeyboard);
  LOCK_VARIABLE(breakf);
  LOCK_VARIABLE(keysprev);
  LOCK_VARIABLE(keysnext);
  LOCK_VARIABLE(keyspressed);
  LOCK_FUNCTION(inportb_locked);

  _go32_dpmi_get_protected_mode_interrupt_vector(9,&oldkeyboard);
  _go32_dpmi_chain_protected_mode_interrupt_vector(9,&newkeyboard);
}

void deinitkeyboard()
{
  _go32_dpmi_set_protected_mode_interrupt_vector(9,&oldkeyboard);
}

#endif /* SOPWITH_DJGPP_KEYBOARD_H */
