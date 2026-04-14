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
#ifndef SOPWITH_SDL_GRAPHICS_H
#define SOPWITH_SDL_GRAPHICS_H

#include <SDL/SDL.h>
#include <map>
#include "../sopwith.h"
#include "../sprites.h"
#include "../sw_excep.h"

int colour_black;
int colour_cyan;
int colour_magenta;
int colour_white;
int colour_green;
int colour_tan;
int colour_brown;
int screen_width;
int screen_height;

SDL_Surface* screen;

std::map<std::string,SDL_Surface*> bitmapmap;

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
  int bpp=surface->format->BytesPerPixel;
  Uint8* p=reinterpret_cast<Uint8*>(surface->pixels)+y*surface->pitch+x*bpp;

  switch(bpp) {
    case 1:
      return *p;
    case 2:
      return *reinterpret_cast<Uint16*>(p);
    case 3:
      if(SDL_BYTEORDER==SDL_BIG_ENDIAN)
        return (p[0]<<16) | (p[1]<<8) | p[2];
      else
        return p[0] | (p[1]<<8) | (p[2]<<16);
    case 4:
      return *reinterpret_cast<Uint32*>(p);
  }
  return 0; /* Placed to suppress warning */
}

inline int cgapixel(unsigned char quad,int index)
{
  int shift=(3-index)<<1;
  switch (((3<<shift)&quad)>>shift) {
    case 0:
      return colour_black;
    case 1:
      return colour_cyan;
    case 2:
      return colour_magenta;
    case 3:
      return colour_white;
  }
  return 0; /* Placed to suppress warning */
}

inline void putpixel_(SDL_Surface* buffer,int x,int y,int c)
{
  if (x<0 || y < 0 || x>=buffer->w || y>=buffer->h)
    return;
  int bpp=buffer->format->BytesPerPixel;
  Uint8* p=reinterpret_cast<Uint8*>(buffer->pixels)+y*buffer->pitch+x*bpp;

  switch(bpp) {
    case 1:
      *p=c;
      break;
    case 2:
      *reinterpret_cast<Uint16*>(p)=c;
      break;
    case 3:
      if (SDL_BYTEORDER==SDL_BIG_ENDIAN) {
        p[0]=(c>>16)&0xff;
        p[1]=(c>>8)&0xff;
        p[2]=c&0xff;
      }
      else {
        p[0]=c&0xff;
        p[1]=(c>>8)&0xff;
        p[2]=(c>>16)&0xff;
      }
      break;
    case 4:
      *reinterpret_cast<Uint32*>(p)=c;
      break;
  }
}

inline std::string base36(unsigned int number)
{
  return std::string()+((number<10)?static_cast<char>('0'+number):static_cast<char>('a'-10+number));
}

void createsprite(std::string baseidentifier,unsigned char* sprite,int width,int height,int a1=0,int a2=0,int a3=0)
{
  std::string bitmapidentifier=baseidentifier+base36(a1)+base36(a2)+base36(a3);
  SDL_Surface* bitmap=SDL_CreateRGBSurface(SDL_SWSURFACE,width*zoomx(),height*zoomy(),screen->format->BitsPerPixel,0,0,0,0);
  SDL_SetColors(bitmap,screen->format->palette->colors,0,256);
  SDL_SetColorKey(bitmap,SDL_SRCCOLORKEY,colour_black);
  if (SDL_MUSTLOCK(bitmap))
    SDL_LockSurface(bitmap);
  width=(width+3)>>2;
  for (int yy=0;yy<height;++yy)
    for (int xx=0;xx<width;++xx) {
      unsigned char byte=reinterpret_cast<unsigned char*>(sprite)[yy*width+xx];
      for (int b=0;b<4;++b) {
        int pixel=cgapixel(byte,b);
        for (int zy=0;zy<zoomy();++zy)
          for (int zx=0;zx<zoomx();++zx)
            putpixel_(bitmap,(xx*4+b)*zoomx()+zx,yy*zoomy()+zy,pixel);
      }
    }
  if (SDL_MUSTLOCK(bitmap))
    SDL_UnlockSurface(bitmap);

  bitmapmap[bitmapidentifier]=bitmap;
}

void graphicsmode()
{
  std::cout<<"Initializing graphics...";

  SDL_WM_SetCaption("Sopwith 3",0);
  SDL_Surface* icon=SDL_LoadBMP("data/images/icon.bmp");
  if (icon!=0) {
    static Uint8 bitmask[32][4];
    int startbit=1<<7;
    if (SDL_MUSTLOCK(icon))
      SDL_LockSurface(icon);
    for (int y=0;y<icon->h;++y)
      for (int x=0;x<icon->w;++x) {
        Uint8 r,g,b;
        SDL_GetRGB(getpixel(icon,x,y),icon->format,&r,&g,&b);
        if (r!=0 || g!=0 || b!=0)
          bitmask[y][x>>3]|=startbit>>(x&7);
      }
    if (SDL_MUSTLOCK(icon))
      SDL_UnlockSurface(icon);
    SDL_WM_SetIcon(icon,reinterpret_cast<Uint8*>(bitmask));
    SDL_FreeSurface(icon);
  }
  if ((screen=SDL_SetVideoMode(requested_screen_width,400,8,SDL_SWSURFACE))==0) {
      std::stringstream exception;
      exception << "SDL: SDL_SetVideoMode(int,int,int,Uint32): " << SDL_GetError();
      throw sw_excep(exception);
  }
  SDL_ShowCursor(SDL_DISABLE);
  colour_black=  SDL_MapRGB(screen->format,0x00,0x00,0x00);
  colour_cyan=   SDL_MapRGB(screen->format,0x00,0xff,0xff);
  colour_magenta=SDL_MapRGB(screen->format,0xff,0x00,0xff);
  colour_white=  SDL_MapRGB(screen->format,0xff,0xff,0xff);
  colour_green=  SDL_MapRGB(screen->format,0x00,0xff,0x00);
  colour_tan=    SDL_MapRGB(screen->format,0xc0,0x80,0x80);
  colour_brown=  SDL_MapRGB(screen->format,0x80,0x40,0x00);
  screen_width=screen->w;
  screen_height=screen->h;
  units_per_screen_width=screen_width/2;
  units_per_screen_height=screen_height/2;

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x8;++a2)
      createsprite("bomb",bombsprites[a1][a2],BOMB_WIDTH,BOMB_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x4;++a2)
      createsprite("targ",targetsprites[a1][a2],TARGET_WIDTH,TARGET_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    createsprite("debr",debrissprites[a1],TARGET_WIDTH,TARGET_HEIGHT,a1);

  for (int a1=0;a1<0x3;++a1)
    for (int a2=0;a2<0x8;++a2)
      createsprite("frag",fragsprites[a1][a2],FRAG_WIDTH,FRAG_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x2;++a2)
      for (int a3=0;a3<0x10;++a3)
        createsprite("plan",planesprites[a1][a2][a3],PLANE_WIDTH,PLANE_HEIGHT,a1,a2,a3);

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x4;++a2)
      createsprite("fina",finalesprites[a1][a2],PLANE_WIDTH,PLANE_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x2;++a2)
      createsprite("fall",fallingsprites[a1][a2],PLANE_WIDTH,PLANE_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    createsprite("floc",flocksprites[a1],FLOCK_WIDTH,FLOCK_HEIGHT,a1);

  for (int a1=0;a1<0x2;++a1)
    createsprite("bird",birdsprites[a1],BIRD_WIDTH,BIRD_HEIGHT,a1);

  for (int a1=0;a1<0x2;++a1)
    createsprite("ox",oxsprites[a1],OX_WIDTH,OX_HEIGHT,a1);

  for (int a1=0;a1<0x3;++a1)
    createsprite("pixe",pixelsprites[a1],1,1,a1);

  std::cout<<"Done.\n";
}

void textmode()
{
  const std::map<std::string,SDL_Surface*>::const_iterator end=bitmapmap.end();
  for (std::map<std::string,SDL_Surface*>::const_iterator cur=bitmapmap.begin();cur!=end;++cur) {
    SDL_FreeSurface(cur->second);
  }
  bitmapmap.clear();
}

void displaytitlescreen()
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  int i=0;
  for (int f=0;f<=0x2000;f+=0x2000) {
    int end=f+8000;
    for (int pos=f;pos<end;) {
      int end2=pos+(titlescreen[i]&0x7f);
      if ((titlescreen[i++]&0x80)!=0) {
        unsigned char quad=titlescreen[i++];
        int pixel[4];
        for (int x=0;x<4;++x)
          pixel[x]=cgapixel(quad,x);
        for (;pos<end2;++pos)
          for (int x=0;x<4;++x)
            for (int zy=0;zy<zoomy();++zy)
              for (int zx=0;zx<zoomx();++zx)
                putpixel_(screen,((((pos%0x2000)<<2)%320)+x)*zoomx()+zx,((((pos%0x2000)/80)<<1)+((pos>=0x2000)?1:0))*zoomy()+zy,pixel[x]);
      }
      else {
        for (;pos<end2;++i,++pos)
          for (int x=0;x<4;++x)
            for (int zy=0;zy<zoomy();++zy)
              for (int zx=0;zx<zoomx();++zx)
                putpixel_(screen,((((pos%0x2000)<<2)%320)+x)*zoomx()+zx,((((pos%0x2000)/80)<<1)+((pos>=0x2000)?1:0))*zoomy()+zy,cgapixel(titlescreen[i],x));
      }
    }
  }
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
  SDL_UpdateRect(screen,0,0,0,0);
}

void cleartitlescreen()
{
  clearscreen();
}

inline void drawchar(int x,int y,int colour,Drawcharmode mode,char character)
{
  switch (mode) {
    case BLACK_BACKGROUND: {
      for (int yy=0;yy<8;++yy) {
        unsigned char temp = s_font[static_cast<int>(character)][yy];
        for (int xx=0;xx<8;++xx)
          if (temp & (1<<xx))
            putpixel_(screen,x+xx,y+yy,colour);
          else
            putpixel_(screen,x+xx,y+yy,colour_black);
      }
    }
    break;

    case TRANSPARENT_BACKGROUND: {
      for (int yy=0;yy<8;++yy) {
        unsigned char temp = s_font[static_cast<int>(character)][yy];
        for (int xx=0;xx<8;++xx)
          if (temp & (1<<xx))
            putpixel_(screen,x+xx,y+yy,colour);
      }
    }
    break;

    default:
    break;
  }
}


void drawstr(int x,int y,int colour,Drawcharmode mode,const char* string)
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  while (*string!=0 && x<screen_width) {
    drawchar(x,y,colour,mode,*(string++));
    x+=8;
  }
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void clearscreen()
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  memset(screen->pixels,0,screen->pitch*screen->h);
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void putpixel(int x,int y,int c)
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  putpixel_(screen,x,screen_height-1-y,c);
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void putimage(int x,int y,void* p,int,int)
{
  SDL_Rect dstrect;
  dstrect.x=x;
  dstrect.y=screen_height-zoomy()-y;
  SDL_BlitSurface(reinterpret_cast<SDL_Surface*>(p),0,screen,&dstrect);
}

void displayscreen()
{
  SDL_UpdateRect(screen,0,0,0,0);
}

void drawmapobject(int x,int y,int c)
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  putpixel_(screen,x,screen_height-1-y,c);
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void initscreen()
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  memset(screen->pixels,0,screen->pitch*screen->h);
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void drawground(ground_t* screenground,int height)
{
  int y=*screenground-height,y2;
  for (int x=0;x<screen_width;) {
    y2=*(screenground++)-height;
    for (int i=0;i<zoomx();++i,++x)
      if (y2==y)
        putpixel_(screen,x,screen_height-(1+y)*zoomy(),colour_white);
      else
        if (y2>y)
          do {
            for (int y3=(1+y)*zoomy();y3<(2+y)*zoomy();++y3)
              putpixel_(screen,x,screen_height-y3,colour_white);
          }
          while (++y!=y2);
        else
          do {
            for (int y3=(1+y)*zoomy();y3>y*zoomy();--y3)
              putpixel_(screen,x,screen_height-y3,colour_white);
          }
          while (--y!=y2);
  }
}

void drawground(bool,int screenleft,int screendown)
{
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
  drawground(ground+screenleft,screendown);
  /**/
  for (int x=0;x<screen_width;++x)
    putpixel(x,(MAX_Y-screendown)*zoomy(),(x&1)==1 ? colour_green : colour_black);
  /**/
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void* spritedata(Spritetype spritetype,int a1,int a2,int a3,int)
{
  std::string identifier;
  switch(spritetype) {
    case BOMB:          identifier="bomb"; break;
    case TARGET:        identifier="targ"; a3=0; break;
    case DEBRIS:        identifier="debr"; break;
    case FRAG:          identifier="frag"; break;
    case PLANE:         identifier="plan"; break;
    case PLANE_FINALE:  identifier="fina"; break;
    case PLANE_FALLING: identifier="fall"; break;
    case FLOCK:         identifier="floc"; break;
    case BIRD:          identifier="bird"; break;
    case OX:            identifier=  "ox"; break;
    case PIXEL:         identifier="pixe"; break;
  }
  return bitmapmap[identifier+base36(a1)+base36(a2)+base36(a3)];
}

#endif /* SOPWITH_SDL_GRAPHICS_H */
