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
#ifndef SOPWITH_DJGPP_JOYSTICK_H
#define SOPWITH_DJGPP_JOYSTICK_H

#include "../graphics.h"
#include "../keyboard.h"
#include <pc.h>

int joykeysprev;
int joykeyspressed;
int joykeysnext;

int xlow,ylow,xhigh,yhigh,joypollmax;

bool calibratejoystick(int& x,int& y,int& i,const char* position)
{
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Move joystick to the");
  drawstr(0*8*zoomx(),21*8*zoomy(),colour_magenta,BLACK_BACKGROUND,position);
  drawstr(0*8*zoomx(),22*8*zoomy(),colour_white,BLACK_BACKGROUND,"and press any key to continue");
  displayscreen();
  while (!inkey())
    processtimerticks();
  i=0;
  x=0;
  y=0;
  outportb(0x201,0);
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Trying to calibrate joystick");
  drawstr(0*8*zoomx(),22*8*zoomy(),colour_white,BLACK_BACKGROUND,"Press any key to abort");
  displayscreen();
  while (!inkey()){
    processtimerticks();
    i=inportb(0x201);
    if ((i&3)==0) {
      y>>=1;
      return true;
    }
    x+=i&1;
    y+=i&2;
    ++i;
  }
  return false;
}

void initjoystick()
{
  int xmid,ymid,cmid,xleft,yleft,cleft,xright,yright,cright;
  if (calibratejoystick(xleft ,yleft ,cleft ,"upper left corner") &&
      calibratejoystick(xright,yright,cright,"bottom right corner") &&
      calibratejoystick(xmid  ,ymid  ,cmid  ,"center")) {
    xlow=(xleft+xmid)/2;
    ylow=(yleft+ymid)/2;
    xhigh=(xmid+xright+1)/2;
    yhigh=(ymid+yright+1)/2;
    joypollmax=std::max(cmid,std::max(cleft,cright));
  }
  else
    controls&=~JOYSTICK;
}

void deinitjoystick()
{
}

int readjoychannel(int& x,int& y)
{
  int t=0;
  x=0;
  y=0;
  outportb(0x201,0);
  for (int i=0;i<joypollmax;++i){
    t=inportb(0x201);
    if ((t&3)==0)
      break;
    x+=t&1;
    y+=t&2;
  }
  y>>=1;
  return t;
}

void updatejoy()
{
  static int joykeys[9]={KEY_DESCEND,KEY_DESCEND,KEY_DESCEND,
                         KEY_FLIP   ,0          ,KEY_FLIP,
                         KEY_CLIMB  ,KEY_CLIMB  ,KEY_CLIMB};

  int x,y,j,r;
  if ((controls&JOYSTICK)!=0) {
    r=readjoychannel(x,y);
    j=joykeys[(x<=xlow ? 0 : (x>=xhigh ? 2 : 1))+
              (y<=ylow ? 0 : (y>=yhigh ? 6 : 3))];
    if ((r&0x10)==0)
      j|=KEY_FIRE;
    if ((r&0x20)==0)
      j|=KEY_BOMB;
    if ((r&0x40)==0)
      j|=KEY_BRAKE;
    if ((r&0x80)==0)
      j|=KEY_ACCEL;
    joykeyspressed=j;
    joykeysnext|=j;
    joykeysnext&=j|~joykeysprev;
  }
}

#endif /* SOPWITH_DJGPP_JOYSTICK_H */
