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
#ifndef SOPWITH_DJGPP_SCREEN_H
#define SOPWITH_DJGPP_SCREEN_H

#include <sys/farptr.h>

class Screenpointer;

class Screen {
  friend class Screenpointer;
public:
  Screen& operator=(unsigned long int value);
  Screen& operator|=(unsigned long int value);
  Screen& operator&=(unsigned long int value);
  Screen& operator^=(unsigned long int value);
  operator unsigned long int();
private:
  Screen();

  unsigned long int pointer;
};

class Screenpointer {
public:
  Screenpointer();
  Screenpointer(unsigned long int p);
  Screenpointer& operator=(unsigned long int p);
  Screenpointer& operator+=(unsigned long int p);
  Screenpointer operator++(int);
  Screenpointer& operator++();
  Screen& operator*();

private:
  Screen s;
};

inline Screen& Screen::operator=(unsigned long int value)
{
  _farnspokeb(pointer,value);
  return *this;
}

inline Screen& Screen::operator|=(unsigned long int value)
{
  _farnspokeb(pointer,_farnspeekb(pointer)|value);
  return *this;
}

inline Screen& Screen::operator&=(unsigned long int value)
{
  _farnspokeb(pointer,_farnspeekb(pointer)&value);
  return *this;
}

inline Screen& Screen::operator^=(unsigned long int value)
{
  _farnspokeb(pointer,_farnspeekb(pointer)^value);
  return *this;
}

inline Screen::operator unsigned long int()
{
  return _farnspeekb(pointer);
}

inline Screen::Screen()
{
}

inline Screenpointer::Screenpointer()
{
}
  
inline Screenpointer::Screenpointer(unsigned long int p)
{
  s.pointer=p;
}

inline Screenpointer& Screenpointer::operator=(unsigned long int p)
{
  s.pointer=p;
  return *this;
}

inline Screenpointer& Screenpointer::operator+=(unsigned long int p)
{
  s.pointer+=p;
  return *this;
}

inline Screenpointer Screenpointer::operator++(int)
{
  return Screenpointer(s.pointer++);
}

inline Screenpointer& Screenpointer::operator++()
{
  ++s.pointer;
  return *this;
}

inline Screen& Screenpointer::operator*() {
  return s;
}

#endif /* SOPWITH_DJGPP_SCREEN_H */
