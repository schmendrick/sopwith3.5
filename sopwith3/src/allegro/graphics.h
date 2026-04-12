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
#ifndef SOPWITH_ALLEGRO_GRAPHICS_H
#define SOPWITH_ALLEGRO_GRAPHICS_H

#include <allegro.h>
#include <map>
#include <string>
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

BITMAP* buffer;
PALETTE palette;

struct Extendedbitmap {
  Extendedbitmap(BITMAP* bitmap,int offsetx,int offsety) :
    bitmap(bitmap),
    offsetx(offsetx),
    offsety(offsety) {}
  ~Extendedbitmap()
  {
    destroy_bitmap(bitmap);
  }
  BITMAP* bitmap;
  int offsetx;
  int offsety;
};

std::map<std::string,Extendedbitmap*> bitmapmap;

BITMAP* spritetobitmap(unsigned char* sprite,int width,int height,bool resize=true)
{
  BITMAP* bitmap=create_bitmap(width,height);
  clear_bitmap(bitmap);
  int widthbytes=(width+3)>>2;
  for (int y=0;y<height;++y)
    for (int x=0;x<width;++x) {
      int shift=(3-(x&3))<<1;
      int cgacolour=((3<<shift)&sprite[y*widthbytes+(x>>2)])>>shift;
      int newcolour;
      switch (cgacolour) {
        case 1:
          newcolour=colour_cyan;
          break;
        case 2:
          newcolour=colour_magenta;
          break;
        case 3:
          newcolour=colour_white;
          break;
        default:
          /* Mask colour */
          continue;
      }
      putpixel(bitmap,x,y,newcolour);
    }
  if (resize) {
    BITMAP* bitmap2=create_bitmap(width*zoomx(),height*zoomy());
    clear_bitmap(bitmap2);
    stretch_sprite(bitmap2,bitmap,0,0,width*zoomx(),height*zoomy());
    destroy_bitmap(bitmap);
    return bitmap2;
  }
  else
    return bitmap;
}

inline std::string base36(unsigned int number)
{
  return std::string()+((number<10)?static_cast<char>('0'+number):static_cast<char>('a'-10+number));
}

void createsprite(std::string baseidentifier,unsigned char* sprite,int width,int height,int a1=0,int a2=0,int a3=0,int a4=0,bool resize=true)
{
  std::string bitmapidentifier=baseidentifier+base36(a1)+base36(a2)+base36(a3)+base36(a4);
  BITMAP* bitmap=load_bitmap(("data/images/"+bitmapidentifier+".pcx").c_str(),palette);
  if (bitmap!=0)
    bitmapmap[bitmapidentifier]=new Extendedbitmap(bitmap,0,height*zoomy()-bitmap->h);
  else {
    bitmap=spritetobitmap(sprite,width,height,resize);
    bitmapmap[bitmapidentifier]=new Extendedbitmap(bitmap,0,0);
    if (complementsprites) {
      get_palette(palette);
      save_bitmap(("data/images/"+bitmapidentifier+".pcx").c_str(),bitmap,palette);
    }
  }
}

void graphicsmode()
{
  if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,480,0,0)<0)
    if (set_gfx_mode(GFX_AUTODETECT,640,480,0,0)<0) {
      std::stringstream exception;
      exception << "Allegro: set_gfx_mode(int,int,int,int,int): " << allegro_error;
      throw sw_excep(exception);
    }
  set_display_switch_mode(SWITCH_BACKGROUND);
  BITMAP* palettebitmap=load_bitmap("data/images/palette.pcx",palette);
  if (palettebitmap!=0)
    set_palette(palette);
  else
    set_palette(default_palette);
  destroy_bitmap(palettebitmap);
  colour_black=  makecol(0x00,0x00,0x00);
  colour_cyan=   makecol(0x00,0xff,0xff);
  colour_magenta=makecol(0xff,0x00,0xff);
  colour_white=  makecol(0xff,0xff,0xff);
  colour_green=  makecol(0x00,0xff,0x00);
  colour_tan=    makecol(0xc0,0x80,0x80);
  colour_brown=  makecol(0x80,0x40,0x00);
  screen_width=SCREEN_W;
  screen_height=SCREEN_H;
  buffer=create_bitmap(screen_width,screen_height);
  clear_bitmap(buffer);

  for (int a1=0;a1<0x2;++a1) {
    createsprite("bomb",bombsprites[a1][0],BOMB_WIDTH,BOMB_HEIGHT,a1,0);
    BITMAP* base=bitmapmap["bomb"+base36(a1)+"000"]->bitmap;
    for (int a2=1;a2<0x8;++a2) {
      BITMAP* bitmap=create_bitmap(BOMB_WIDTH*zoomx(),BOMB_HEIGHT*zoomy());
      clear_bitmap(bitmap);
      rotate_sprite(bitmap,base,0,0,-itofix(a2<<5));
      bitmapmap["bomb"+base36(a1)+base36(a2)+"00"]=new Extendedbitmap(bitmap,0,0);
    }
  }

  for (int a1=0;a1<0x2;++a1)
    for (int a2=0;a2<0x4;++a2)
      for (int a3=0;a3<0x4;++a3)
        createsprite("targ",targetsprites[a1][a2],TARGET_WIDTH,TARGET_HEIGHT,a1,a2,a3);

  for (int a1=0;a1<0x2;++a1)
    createsprite("debr",debrissprites[a1],TARGET_WIDTH,TARGET_HEIGHT,a1);

  for (int a1=0;a1<0x3;++a1)
    for (int a2=0;a2<0x8;++a2)
      createsprite("frag",fragsprites[a1][a2],FRAG_WIDTH,FRAG_HEIGHT,a1,a2);

  for (int a1=0;a1<0x2;++a1)
    for (int a4=0;a4<0x2;++a4) {
      createsprite("plan",planesprites[a1][0][0],PLANE_WIDTH,PLANE_HEIGHT,a1,0,0,a4);
      BITMAP* base=bitmapmap["plan"+base36(a1)+"00"+base36(a4)]->bitmap;
      for (int a3=1;a3<0x10;++a3) {
        BITMAP* bitmap=create_bitmap(PLANE_WIDTH*zoomx(),PLANE_HEIGHT*zoomy());
        clear_bitmap(bitmap);
        rotate_sprite(bitmap,base,0,0,-itofix(a3<<4));
        bitmapmap["plan"+base36(a1)+"0"+base36(a3)+base36(a4)]=new Extendedbitmap(bitmap,0,0);
      }
      for (int a3=0;a3<0x10;++a3) {
        BITMAP* bitmap=create_bitmap(PLANE_WIDTH*zoomx(),PLANE_HEIGHT*zoomy());
        clear_bitmap(bitmap);
        rotate_sprite_v_flip(bitmap,base,0,0,-itofix(a3<<4));
        bitmapmap["plan"+base36(a1)+"1"+base36(a3)+base36(a4)]=new Extendedbitmap(bitmap,0,0);
      }
    }

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
    createsprite("pixe",pixelsprites[a1],1,1,a1,0,0,0,false);
}

void textmode()
{
  const std::map<std::string,Extendedbitmap*>::const_iterator end=bitmapmap.end();
  for (std::map<std::string,Extendedbitmap*>::const_iterator cur=bitmapmap.begin();cur!=end;++cur) {
    delete cur->second;
  }
  bitmapmap.clear();

  destroy_bitmap(buffer);
  set_gfx_mode(GFX_TEXT,0,0,0,0);
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

void displaytitlescreen()
{
  BITMAP* titlebitmap=create_bitmap(units_per_screen_width,units_per_screen_height);
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
            putpixel(titlebitmap,(((pos%0x2000)<<2)%320)+x,(((pos%0x2000)/80)<<1)+((pos>=0x2000)?1:0),pixel[x]);
      }
      else {
        for (;pos<end2;++i,++pos)
          for (int x=0;x<4;++x)
            putpixel(titlebitmap,(((pos%0x2000)<<2)%320)+x,(((pos%0x2000)/80)<<1)+((pos>=0x2000)?1:0),cgapixel(titlescreen[i],x));
      }
    }
  }
  /* get_palette(palette); */
  /* set_palette(black_palette); */
  stretch_blit(titlebitmap,screen,0,0,titlebitmap->w,titlebitmap->h,0,0,screen_width,screen_height);
  /* fade_in(palette,1); */
  destroy_bitmap(titlebitmap);
}

void cleartitlescreen()
{
  /* fade_out(1); */
  clear_bitmap(screen);
  /* set_palette(palette); */
}

void drawstr(int x,int y,int colour,Drawcharmode mode,const char* string)
{
  if (mode==BLACK_BACKGROUND)
    text_mode(colour_black);
  else
    text_mode(-1);
  textout(buffer,font,string,x,y,colour);
}

void clearscreen()
{
  /*clear_bitmap(screen);*/
}

void putpixel(int x,int y,int c)
{
  putpixel(buffer,x,screen_height-1-y,c);
}

void putimage(int x,int y,void* p,int,int)
{
  masked_blit(reinterpret_cast<Extendedbitmap*>(p)->bitmap,buffer,0,0,x+reinterpret_cast<Extendedbitmap*>(p)->offsetx,screen_height-zoomy()-y+reinterpret_cast<Extendedbitmap*>(p)->offsety,reinterpret_cast<Extendedbitmap*>(p)->bitmap->w*zoomx(),reinterpret_cast<Extendedbitmap*>(p)->bitmap->h*zoomy());
}

void displayscreen()
{
  blit(buffer,screen,0,0,0,0,screen_width,screen_height);
}

void drawmapobject(int x,int y,int c)
{
  putpixel(buffer,x,screen_height-1-y,c);
}

void initscreen()
{
  clear_bitmap(buffer);
}

inline void drawground(ground_t* screenground,int height)
{
  /*BITMAP* groundbuffer=create_bitmap(screen_width,screen_height);
  clear_bitmap(groundbuffer);*/
  int y=*screenground-height,y2;
  for (int x=0;x<screen_width;) {
    y2=*(screenground++)-height;
    for (int i=0;i<zoomx();++i,++x)
      if (y2==y)
        putpixel(buffer,x,screen_height-(1+y)*zoomy(),colour_white);
      else
        if (y2>y)
          do {
            for (int y3=(1+y)*zoomy();y3<(2+y)*zoomy();++y3)
              putpixel(buffer,x,screen_height-y3,colour_white);
          }
          while (++y!=y2);
        else
          do {
            for (int y3=(1+y)*zoomy();y3>y*zoomy();--y3)
              putpixel(buffer,x,screen_height-y3,colour_white);
          }
          while (--y!=y2);
  }
  /*stretch_sprite(buffer,groundbuffer,0,screen_height-units_per_screen_height*zoomy(),units_per_screen_width*zoomx(),units_per_screen_height*zoomy());
  destroy_bitmap(groundbuffer);*/
}

void drawground(bool,int screenleft,int screendown)
{
  drawground(ground+screenleft,screendown);
  /**/
  for (int x=0;x<screen_width;++x)
    putpixel(x,(MAX_Y-screendown)*zoomy(),(x&1)==1 ? colour_green : colour_black);
  /**/
}

void* spritedata(Spritetype spritetype,int a1,int a2,int a3,int a4)
{
  std::string identifier;
  switch(spritetype) {
    case BOMB:          identifier="bomb"; break;
    case TARGET:        identifier="targ"; break;
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
  return bitmapmap[identifier+base36(a1)+base36(a2)+base36(a3)+base36(a4)];
}

#endif /* SOPWITH_ALLEGRO_GRAPHICS_H */
