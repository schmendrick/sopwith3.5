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
#ifndef SOPWITH_DJGPP_TIMER_H
#define SOPWITH_DJGPP_TIMER_H

#include "misc.h"

namespace {
  _go32_dpmi_seginfo oldtimer;

  volatile unsigned long timertick=0;

  void timerint()
  {
    ++timertick;
  }
  END_OF_FUNCTION(timerint);
}

void inittimer()
{
  _go32_dpmi_seginfo newtimer;

  newtimer.pm_selector=_go32_my_cs();
  newtimer.pm_offset=reinterpret_cast<int>(timerint);

  LOCK_FUNCTION(timerint);
  LOCK_VARIABLE(timertick);

  _go32_dpmi_get_protected_mode_interrupt_vector(8,&oldtimer);
  _go32_dpmi_chain_protected_mode_interrupt_vector(8,&newtimer);
}

unsigned long timer()
{
  return timertick;
}

void timeridle()
{
}

void deinittimer()
{
  _go32_dpmi_set_protected_mode_interrupt_vector(8,&oldtimer);
}

#endif /* SOPWITH_DJGPP_TIMER_H */
