/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2001 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

#ifndef _SOPWITH_SOPASM_H
#define _SOPWITH_SOPASM_H

#ifdef __cplusplus
extern "C" {
#endif

void farmemmove(void far *dest,void far *src,int length);
void farmemset(void far *p,char fill,int length);
void writechar(char c);
void clearline(void);
void poscurs(int x,int y);
void setgmode(int m);
void inittimer(void (*f)(void));
void initkeyboard(void (*f)(void));
void restoreints(void);
unsigned int getwordfromport(int port);
int getkey(void);
bool kbhit(void);

#ifdef __cplusplus
}
#endif

#endif /* _SOPWITH_SOPASM_H */
