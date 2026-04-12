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
#ifndef SOPWITH_SW_EXCEP_H
#define SOPWITH_SW_EXCEP_H

#include <sstream>
#include <string>

class sw_excep : public std::exception {
public:
  sw_excep(const std::stringstream& rhs) : what_(rhs.str()) {}
  sw_excep(const char* rhs)              : what_(rhs) {}
  template<class T> sw_excep(const T& rhs)
  {
    std::stringstream s;
    s << rhs;
    what_=s.str();
  }
  virtual ~sw_excep() throw() {}
  virtual const char* what() const throw()
  {
    return what_.c_str();
  }
private:
  std::string what_;
};

#endif /* SOPWITH_SW_EXCEP_H */
