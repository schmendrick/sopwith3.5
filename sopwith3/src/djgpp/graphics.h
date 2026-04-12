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
#ifndef SOPWITH_DJGPP_GRAPHICS_H
#define SOPWITH_DJGPP_GRAPHICS_H

#include "screen.h"
#include <dpmi.h>
#include <string>
#include <sys/farptr.h>
#include <sys/segments.h>
#include <map>
#include <list>
#include "../sprites.h"
#include "../sopwith.h"
#include "../sw_excep.h"

int colour_black=0;
int colour_cyan=1;
int colour_magenta=2;
int colour_white=3;
int colour_green=3;
int colour_tan=1;
int colour_brown=3;
int screen_width=320;
int screen_height=200;

namespace {
  struct Screensprite {
    Screensprite(int x,int y,void* p,int width,int height) :
      x(x),
      y(y),
      p(p),
      width(width),
      height(height) {}
    int x,y;
    void* p;
    int width,height;
  };

  enum {tobexored=1,isonscreen=2};

  struct lss {
    inline bool operator()(const Screensprite& s1,const Screensprite& s2)
    {
      return (s1.x<s2.x) || ((s1.x==s2.x) &&
                             ((s1.y<s2.y) || ((s1.y==s2.y) &&
                                              (s1.p<s2.p))));
    }
  };

  std::map<Screensprite,int,lss> spritemap;

  struct Mappixel {
    Mappixel(int x,int y,int colour) :
      x(x),
      y(y),
      colour(colour) {}
    int x,y,colour;
  };

  std::list<Mappixel> maplist;
  bool groundtobedrawn=true;
  ground_t screenground[320];
}

void graphicsmode()
{
  int CGASelector=__dpmi_allocate_ldt_descriptors(1);
  static char selectorData[8]={
    0xff,0x3f,0x00,0x80,
    0x0b,0xf3,0x40,0x00
  };

  if (__dpmi_set_descriptor(CGASelector,selectorData)<0)
    throw sw_excep("DJGPP: Function __dpmi_set_descriptor(int,void*) failed");

  _farsetsel(CGASelector);

  __dpmi_regs r;
  r.x.ax=4;
  __dpmi_int(0x10,&r);
}

void textmode()
{
  __dpmi_regs r;
  r.x.ax=3;
  __dpmi_int(0x10,&r);
}

namespace {
  inline void memset_djgpp(unsigned int offset,unsigned char c,unsigned int length)
  {
    unsigned int pastlastbyte=offset+length;
    unsigned int pastlastdoubleword=pastlastbyte-3;
    unsigned long doubleword=c*0x01010101;
    for (;offset<pastlastdoubleword;offset+=4)
      _farnspokel(offset,doubleword);
    for (;offset<pastlastbyte;++offset)
      _farnspokeb(offset,c);
  }
}

void displaytitlescreen()
{
  int i=0;
  for (int f=0;f<=0x2000;f+=0x2000) {
    int end=f+8000;
    for (int pos=f;pos<end;) {
      int run=titlescreen[i]&0x7f;
      if ((titlescreen[i++]&0x80)!=0)
        memset_djgpp(pos,titlescreen[i++],run);
      else {
        movedata(_my_ds(),reinterpret_cast<unsigned int>(&titlescreen[i]),_fargetsel(),pos,run);
        i+=run;
      }
      pos+=run;
    }
  }
}

void cleartitlescreen()
{
  clearscreen();
}

inline void drawchar(int x,int y,int colour,Drawcharmode mode,char character)
{
  static const unsigned char clearpixel[4]={0x3f,0xcf,0xf3,0xfc};
  static const unsigned char drawpixel[4][4]={
    {0x00,0x40,0x80,0xc0},
    {0x00,0x10,0x20,0x30},
    {0x00,0x04,0x08,0x0c},
    {0x00,0x01,0x02,0x03}};

  const unsigned int after_last_x = (x>312) ? (320-x) : 8;
  const unsigned int after_last_y = (y>192) ? (200-y) : 8;
  unsigned int first_x,first_y,startpixel,width;
  Screenpointer ptr;
  int linediff=0x2000;

  if (x<0) {
    first_x = -x;
    x = 0;
  }
  else {
    first_x = 0;
  }

  startpixel = x&3;
  width = (startpixel+after_last_x-first_x)>>2;

  if (y<0) {
    first_y = -y;
    y = 0;
  }
  else {
    first_y = 0;
  }

  ptr=(y>>1)*80+(x>>2);

  if ((y&1)!=0) {
    ptr+=linediff;
    linediff=80-linediff;
  }

  switch (mode) {
    case BLACK_BACKGROUND: {
      for (;first_y<after_last_y;++first_y) {
        unsigned int pixel = startpixel;
        unsigned char temp = s_font[character][first_y];
        unsigned int xx=first_x;
        for (;xx<after_last_x;++xx) {
          *ptr &= clearpixel[pixel];
          if (temp & (1<<xx)) {
            *ptr |= drawpixel[pixel][colour];
          }
          if (pixel==3) {
            pixel=0;
            ++ptr;
          }
          else {
            ++pixel;
          }
        }
        ptr+=linediff-width;
        linediff=80-linediff;
      }
    }
    break;

    case TRANSPARENT_BACKGROUND: {
      for (;first_y<after_last_y;++first_y) {
        unsigned int pixel = startpixel;
        unsigned char temp = s_font[character][first_y];
        unsigned int xx=first_x;
        for (;xx<after_last_x;++xx) {
          if (temp & (1<<xx)) {
            *ptr &= clearpixel[pixel];
            *ptr |= drawpixel[pixel][colour];
          }
          if (pixel==3) {
            pixel=0;
            ++ptr;
          }
          else {
            ++pixel;
          }
        }
        ptr+=linediff-width;
        linediff=80-linediff;
      }
    }
    break;

    case XOR_CHARACTER: {
      for (;first_y<after_last_y;++first_y) {
        unsigned int pixel = startpixel;
        unsigned char temp = s_font[character][first_y];
        unsigned int xx=first_x;
        for (;xx<after_last_x;++xx) {
          if (temp & (1<<xx)) {
            *ptr ^= drawpixel[pixel][colour];
          }
          if (pixel==3) {
            pixel=0;
            ++ptr;
          }
          else {
            ++pixel;
          }
        }
        ptr+=linediff-width;
        linediff=80-linediff;
      }
    }
    break;

    default:
    break;
  }
}

void drawstr(int x,int y,int colour,Drawcharmode mode,const char* string)
{
  while (*string!=0 && x<screen_width) {
    drawchar(x,y,colour,mode,*(string++));
    x+=8;
  }
}

void clearscreen()
{
  memset_djgpp(0,0,8000);
  memset_djgpp(0x2000,0,8000);
  maplist.clear();
  spritemap.clear();
  groundtobedrawn=true;
}

void putpixel(int x,int y,int c)
{
  int shift=(3-(x&3))<<1;
  Screenpointer scp=((199-y)>>1)*80+(x>>2)+((y&1)==0 ? 0x2000 : 0);
  *scp=((*scp)&(~(3<<shift)))|((c&3)<<shift);
}

void xorpixel(int x,int y,int c)
{
  if (y>=0) {
    Screenpointer scp=((199-y)>>1)*80+(x>>2)+((y&1)==0 ? 0x2000 : 0);
    *scp^=(c<<((3-(x&3))<<1));
  }
}

void xorimage(int x,int y,unsigned char* p,int w,int h)
{
  int linediff; unsigned int bltreg;
  Screenpointer scp;
  int width,height,border;
  unsigned char shift,bltleft;
  int leftclip=0;
  shift=8-((x&3)<<1);
  width=(w+3)>>2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  height=(h>y+1 ? y+1 : h);
  if (y>199) {
    height-=y-199;
    p+=width*(y-199);
    y=199;
  }
  if (x<0) {
    leftclip=-x;
    width-=(leftclip+3)>>2;
    x=0;
  }
  scp=((199-y)>>1)*80+(x>>2);
  linediff=0x2000;
  if ((y&1)==0) {
    scp+=linediff;
    linediff=80-linediff;
  }
  do {
    if ((leftclip&3)!=0) {
      p+=(leftclip-1)>>2;
      (*scp)^=(*p)<<((leftclip&3)<<1);
      ++p;
    }
    else
      p+=leftclip>>2;
    bltleft=0;
    for (x=0;x<width;++x) {
      bltreg=((*p++)<<shift)|(bltleft<<8);
      *(scp++)^=(bltreg>>8);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft!=0)
        (*scp)^=bltleft;
    scp+=linediff-width;
    linediff=80-linediff;
    --height;
  } while (height!=0);
}

void putimage(int x,int y,void* p,int w,int h)
{
  int& actions=spritemap[Screensprite(x,y,p,w,h)];
  if ((actions&isonscreen)==0)
    actions=tobexored;
  else
    actions&=~tobexored;
}

void displayscreen()
{
  const std::map<Screensprite,int,lss>::const_iterator end=spritemap.end();
  for (std::map<Screensprite,int,lss>::iterator cur=spritemap.begin();cur!=end;) {
    if ((cur->second&tobexored)!=0)
    {
      Screensprite ss=cur->first;
      xorimage(ss.x,ss.y,reinterpret_cast<unsigned char*>(ss.p),ss.width,ss.height);
      cur->second^=isonscreen;
    }
    if ((cur->second&isonscreen)!=0) {
      cur->second|=tobexored;
      ++cur;
    }
    else
      spritemap.erase(cur++);
  }
}

int pixel(int x,int y)
{
  int shift=(3-(x&3))<<1;
  return ((3<<shift)&(*Screenpointer(((199-y)>>1)*80+(x>>2)+((y&1)==0 ? 0x2000 : 0))))>>shift;
}

void drawmapobject(int x,int y,int c)
{
  maplist.push_back(Mappixel(x,y,c^pixel(x,y)));
  putpixel(x,y,c);
  #if 0
  static unsigned char pixelsprites[0x4][0x1]={{0x00},{0x40},{0x80},{0xc0}};
  putimage(x,y,pixelsprites[colour],1,1);
  const std::map<Screenpos,Screenpixel,lsp>::iterator i=pixelmap.find(Screenpos(x,y));
  if (i==pixelmap.end()) {
    pixelmap[Screenpos(x,y)]=Screenpixel(pixel(x,y),false);
    putpixel(x,y,colour);
  }
  else
    if (i->second.tobedeleted) {
      i->second=Screenpixel(i->second.oldcolour,false);
      putpixel(x,y,colour);
    }
  #endif
}
#if 0
void deletemapobjects()
{
  const std::map<Screenpos,Screenpixel,lsp>::const_iterator end=pixelmap.end();
  for (std::map<Screenpos,Screenpixel,lsp>::iterator cur=pixelmap.begin();cur!=end;) {
    if (cur->second.tobedeleted) {
      putpixel(cur->first.x,cur->first.y,cur->second.oldcolour);
      pixelmap.erase(cur++);
    }
    else {
      cur->second.tobedeleted=true;
      ++cur;
    }
  }
}
#endif

void initscreen()
{
  const std::list<Mappixel>::const_iterator end=maplist.end();
  for (std::list<Mappixel>::const_iterator cur=maplist.begin();cur!=end;++cur) {
    xorpixel(cur->x,cur->y,cur->colour);
  }
  maplist.clear();
}

inline void drawground(ground_t* screenground,int height)
{
  int y=*screenground-height,y2;
  for (int x=0;x<screen_width;++x) {
    y2=*(screenground++)-height;
    if (y2==y)
      xorpixel(x,y,colour_white);
    else
      if (y2>y)
        do
          xorpixel(x,y++,colour_white);
        while (y!=y2);
      else
        do
          xorpixel(x,y--,colour_white);
        while (y!=y2);
  }
}

void drawground(bool forceredraw,int screenleft,int screendown)
{
  static int oldscreenleft,oldscreendown;
  if (groundtobedrawn || forceredraw || oldscreenleft!=screenleft || oldscreendown!=screendown) {
    if (!groundtobedrawn) /**/{/**/
      drawground(screenground,oldscreendown);
      /**/
      for (int x=0;x<screen_width;++x)
        xorpixel(x,MAX_Y-oldscreendown,(x&1)==1 ? colour_green : colour_black);

    }
      /**/
    oldscreenleft=screenleft;
    oldscreendown=screendown;
    memcpy(screenground,ground+oldscreenleft,screen_width*sizeof(ground_t));
    drawground(screenground,oldscreendown);
    /**/
    for (int x=0;x<screen_width;++x)
      xorpixel(x,MAX_Y-oldscreendown,(x&1)==1 ? colour_green : colour_black);
    /**/
  }
  groundtobedrawn=false;
}

void* spritedata(Spritetype spritetype,int a1,int a2,int a3,int)
{
  switch(spritetype) {
    case BOMB:          return    bombsprites[a1][a2];
    case TARGET:        return  targetsprites[a1][a2];
    case DEBRIS:        return  debrissprites[a1];
    case FRAG:          return    fragsprites[a1][a2];
    case PLANE:         return   planesprites[a1][a2][a3];
    case PLANE_FINALE:  return  finalesprites[a1][a2];
    case PLANE_FALLING: return fallingsprites[a1][a2];
    case FLOCK:         return   flocksprites[a1];
    case BIRD:          return    birdsprites[a1];
    case OX:            return      oxsprites[a1];
    case PIXEL:         return   pixelsprites[a1];
  }
  return 0; /* Placed to suppress warning */
}

#endif /* SOPWITH_DJGPP_GRAPHICS_H */
