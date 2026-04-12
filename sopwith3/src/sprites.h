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
#ifndef SOPWITH_SPRITES_H
#define SOPWITH_SPRITES_H

extern unsigned char bombsprites[0x2][0x8][0x10];
extern unsigned char targetsprites[0x2][0x4][0x40];
extern unsigned char debrissprites[0x2][0x40];
extern unsigned char fragsprites[0x3][0x8][0x10];
extern unsigned char planesprites[0x2][0x2][0x10][0x40];
extern unsigned char finalesprites[0x2][0x4][0x40];
extern unsigned char fallingsprites[0x2][0x2][0x40];
extern unsigned char flocksprites[0x2][0x40];
extern unsigned char birdsprites[0x2][0x2];
extern unsigned char oxsprites[0x2][0x40];
extern unsigned char ghostsprites[0x2][0x10];
extern unsigned char shotsprite[0x40];
extern unsigned char splatsprite[0x100];
extern unsigned char missilesprites[0x2][0x10][0x10];
extern unsigned char starburstsprites[0x2][0x2][0x10];
extern unsigned char pixelsprites[0x3][0x1];

#endif /* SOPWITH_SPRITES_H */
