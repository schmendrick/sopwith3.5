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
#ifndef SOPWITH_DJGPP_MISC_H
#define SOPWITH_DJGPP_MISC_H

#include <dpmi.h>
#include <go32.h>

#define END_OF_FUNCTION(x) void x##_end() {}
#define LOCK_VARIABLE(x) _go32_dpmi_lock_data((void*)&x,sizeof(x))
#define LOCK_FUNCTION(x) _go32_dpmi_lock_code((void*)x,(long)x##_end-(long)x)

#endif /* SOPWITH_DJGPP_MISC_H */
