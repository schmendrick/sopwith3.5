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
#ifndef SOPWITH_SDL_MESSAGE_H
#define SOPWITH_SDL_MESSAGE_H

#include "../keyboard.h"
#include <iostream>
#include <string>
#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#endif

inline void message_write_stderr(const std::string& s)
{
#ifdef _WIN32
  std::fwrite(s.data(), 1, s.size(), stderr);
  std::fputc('\n', stderr);
  std::fflush(stderr);
#else
  std::cerr << s << std::endl;
#endif
}

void message(const std::string& s)
{
#ifdef _WIN32
  static bool io_rebound = false;
  if (!io_rebound) {
    /* SDL may disconnect CRT stdio from the parent console. If we already have a console,
       AttachConsole(ATTACH_PARENT_PROCESS) fails with ERROR_ACCESS_DENIED — still freopen
       CONOUT$/CONIN$ so stderr/stdin work again. */
    if (GetConsoleWindow() == nullptr) {
      if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        (void)AllocConsole();
      }
    }
    (void)freopen("CONOUT$", "w", stdout);
    (void)freopen("CONOUT$", "w", stderr);
    (void)freopen("CONIN$", "r", stdin);
    std::ios::sync_with_stdio(true);
    io_rebound = true;
  }
#endif
  message_write_stderr(s);
  std::string temp;
  getline(std::cin, temp);
}

#endif /* SOPWITH_SDL_MESSAGE_H */
