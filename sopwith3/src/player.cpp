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
#ifndef SOPWITH_PLAYER_CPP
#define SOPWITH_PLAYER_CPP

#include "player.h"
#include "keyboard.h"
#include "sopwith.h"
#include "joystick.h"
#include "soundsys.h"

Playerplane::Playerplane(int runwayx,int colour,bool runwayleft,bool unlimitedlives,int maxlives,bool infinitefuel,bool infiniteammo,bool infinitebombs) :
  Plane(runwayx,colour,runwayleft,infinitefuel,infiniteammo,infinitebombs),
  unlimitedlives(unlimitedlives)
{
  init();
  Plane::maxlives=maxlives;
  lives=maxlives;
}

void Playerplane::init()
{
  Plane::init();
  flushkeybuf();
}

void Playerplane::reinit()
{
  init();
  initdisplay();
}

bool Playerplane::update()
{
  if (endstatus!=NOTFINISHED) {
    if (--endcounter<=0) {
      if (gamemode!=MULTIPLE && !exiting)
        gamestatus=RESTARTING;
      else
        gamestatus=EXITING;
    }
  }
  if (latencycount==0) {
    int keys;
    if (gamemode==MULTIPLE)
      keys=lastkey;
    else {
      keys=inkeys();
      flushkeybuf();
    }
    #ifdef DEBUG
    errorfile<<'['<<keys<<']';
    #endif
    processkeys(keys);
  }
  else {
    flaps=0;
    bombing=false;
  }
  if (state==CRASHED && hitcounter<=0) {
    if (!unlimitedlives)
      --lives;
    if (endstatus!=WINNER && (life<=QUIT || (gamemode!=MULTIPLE && lives<=0))) {
      if (endstatus==NOTFINISHED)
        loser(this);
    }
    else {
      reinit();
      if (endstatus==WINNER)
        winner(this);
    }
  }
  bool result=Plane::update();
  if (!athome) {
    if (firing)
      displayammogauge(this);
    if (bombing)
      displaybombgauge(this);
  }
  return result;
}

void Playerplane::processkeys(int keys)
{
  flaps=0;
  bombing=false;
  firing=false;
  if (state==CRASHED)
    return;
  if (state!=FALLING) {
    if (endstatus!=NOTFINISHED) {
      if (endstatus==LOSER)
        gohome();
      return;
    }
    if ((keys&KEY_BREAK)!=0) {
      life=QUIT;
      goinghome=false;
      if (athome) {
        state=CRASHED;
        hitcounter=0;
      }
      exiting=true;
    }
    if ((keys&KEY_GOHOME)!=0 && flying())
      goinghome=true;
  }
  if ((framecounter&1)==1 || !wounded()) {
    if ((keys&KEY_CLIMB)!=0) {
      ++flaps;
      goinghome=false;
    }
    if ((keys&KEY_DESCEND)!=0) {
      --flaps;
      goinghome=false;
    }
    if ((keys&KEY_FLIP)!=0) {
      inverted=!inverted;
      goinghome=false;
    }
    if ((keys&KEY_BRAKE)!=0) {
      if (accel)
        --accel;
      goinghome=false;
    }
    if ((keys&KEY_ACCEL)!=0) {
      if (accel<MAX_THROTTLE)
        ++accel;
      goinghome=false;
    }
  }
  if ((keys&KEY_FIRE)!=0)
    firing=true;
  if ((keys&KEY_BOMB)!=0)
    bombing=true;
  if ((keys&KEY_SOUND)!=0) {
    if (soundflag) {
      clearsounds();
    }
    soundflag=!soundflag;
  }
  if (goinghome)
    gohome();
}

int inkeys()
{
  int k;
  if (ibmkeyboard) {
    pollkeyboard();
    k=keysprev=keysnext;
    keysnext=keyspressed;
    flushkeybuf();
  }
  else {
    switch(inkey()) {
      case 'X': case 'x': k=KEY_ACCEL;     break;
      case 'Z': case 'z': k=KEY_BRAKE;     break;
      case '<': case ',': k=KEY_CLIMB;     break;
      case '?': case '/': k=KEY_DESCEND;   break;
      case '>': case '.': k=KEY_FLIP;      break;
      case ' ':           k=KEY_FIRE;      break;
      case 'B': case 'b': k=KEY_BOMB;      break;
      case 'H': case 'h': k=KEY_GOHOME;    break;
      case 'S': case 's': k=KEY_SOUND;     break;
      case 'P': case 'p': k=KEY_PAUSEGAME; break;
      /**/case '\x1b'   : k=KEY_BREAK;     break;/**/
      default: k=0;
    }
  }
  if ((controls&JOYSTICK)!=0) {
    joykeysprev=joykeysnext;
    joykeysnext=joykeyspressed;
    k|=joykeysprev;
  }
  return history(k);
}

void Playerplane::refuel()
{
  if (topup(life,MAX_FUEL))
    displayfuelgauge(this);
  if (topup(ammo,MAX_AMMO))
    displayammogauge(this);
  if (topup(bombs,MAX_BOMBS))
    displaybombgauge(this);
}

void Playerplane::burnfuel()
{
  if (speed>life%(MAX_FUEL/GAUGE_HEIGHT))
    displayfuelgauge(this);
  Plane::burnfuel();
}

Playerplane* player;

#endif /* SOPWITH_PLAYER_CPP */
