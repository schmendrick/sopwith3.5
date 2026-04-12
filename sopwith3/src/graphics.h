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
#ifndef SOPWITH_GRAPHICS_H
#define SOPWITH_GRAPHICS_H

#include "def.h"

extern int colour_black;
extern int colour_cyan;
extern int colour_magenta;
extern int colour_white;
extern int colour_green;
extern int colour_tan;
extern int colour_brown;
extern int screen_width;
extern int screen_height;

void graphicsmode();
void textmode();
void displaytitlescreen();
void cleartitlescreen();
void drawstr(int x,int y,int colour,Drawcharmode mode,const char* string);
void clearscreen();
void putpixel(int x,int y,int c);
void putimage(int x,int y,void* p,int w,int h);
void displayscreen();
void drawmapobject(int x,int y,int c);
void initscreen();
void drawground(bool forceredraw,int screenleft,int screendown);
void* spritedata(Spritetype spritetype,int a1=0,int a2=0,int a3=0,int a4=0);

#endif /* SOPWITH_GRAPHICS_H */
