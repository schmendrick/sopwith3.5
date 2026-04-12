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
#ifndef SOPWITH_S_FIX_H
#define SOPWITH_S_FIX_H

#include "def.h"

class S_fix {
public:
  inline S_fix() {}

  inline explicit S_fix(int i,unsigned short f=0) :
    integer(i),
    frac(f) {}

  inline S_fix& operator=(int rhs)
  {
    integer=rhs;
    frac=0;
    return *this;
  }

  inline S_fix& operator+=(const S_fix& rhs)
  {
    long temp=(((static_cast<long>(integer))<<16)+frac)+
              (((static_cast<long>(rhs.integer))<<16)+rhs.frac);
    integer=static_cast<short>(temp>>16);
    frac=static_cast<short>(temp);
    return *this;
  }

  inline S_fix& operator-=(const S_fix& rhs)
  {
    long temp=(((static_cast<long>(integer))<<16)+frac)-
              (((static_cast<long>(rhs.integer))<<16)+rhs.frac);
    integer=static_cast<short>(temp>>16);
    frac=static_cast<short>(temp);
    return *this;
  }

  inline S_fix& operator++()
  {
    ++integer;
    return *this;
  }

  inline S_fix& operator--()
  {
    --integer;
    return *this;
  }

  inline friend S_fix operator+(const S_fix& lhs,const S_fix& rhs);
  inline friend S_fix operator+(const S_fix& lhs,int rhs);
  inline friend S_fix operator+(int lhs,const S_fix& rhs);
  inline friend S_fix operator-(const S_fix& lhs,const S_fix& rhs);
  inline friend S_fix operator-(const S_fix& lhs,int rhs);
  inline friend S_fix operator-(int lhs,const S_fix& rhs);
public:
  int integer;
  unsigned short frac;
};

inline S_fix operator+(const S_fix& lhs,const S_fix& rhs)
{
  long temp=(((static_cast<long>(lhs.integer))<<16)+lhs.frac)+
            (((static_cast<long>(rhs.integer))<<16)+rhs.frac);
  return S_fix(static_cast<short>(temp>>16),static_cast<short>(temp));
}

inline S_fix operator+(const S_fix& lhs,int rhs)
{
  return S_fix(lhs.integer+rhs,lhs.frac);
}

inline S_fix operator+(int lhs,const S_fix& rhs)
{
  return S_fix(lhs+rhs.integer,rhs.frac);
}

inline S_fix operator-(const S_fix& lhs,const S_fix& rhs)
{
  long temp=(((static_cast<long>(lhs.integer))<<16)+lhs.frac)-
            (((static_cast<long>(rhs.integer))<<16)+rhs.frac);
  return S_fix(static_cast<short>(temp>>16),static_cast<short>(temp));
}

inline S_fix operator-(const S_fix& lhs,int rhs)
{
  return S_fix(lhs.integer-rhs,lhs.frac);
}

inline S_fix operator-(int lhs,const S_fix& rhs)
{
  return S_fix(lhs-rhs.integer,rhs.frac);
}

namespace {
  int sine[ANGLES]={
    0x000, 0x062, 0x0b5, 0x0ed, 0x100, 0x0ed, 0x0b5, 0x062,
    0x000,-0x062,-0x0b5,-0x0ed,-0x100,-0x0ed,-0x0b5,-0x062};
  inline S_fix s_cos(int r,int a)
  {
    int temp=r*sine[(a+ANGLES/4)&(ANGLES-1)];
    return S_fix(temp>>8,temp<<8);
  }
  inline S_fix s_sin(int r,int a)
  {
    int temp=r*sine[a&(ANGLES-1)];
    return S_fix(temp>>8,temp<<8);
  }
}

#endif /* SOPWITH_S_FIX_H */
