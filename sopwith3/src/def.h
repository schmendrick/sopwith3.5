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
#ifndef SOPWITH_DEF_H
#define SOPWITH_DEF_H

#include <iostream>
#include <string>
#include <sstream>
#ifdef DEBUG
#include <typeinfo>
#endif

enum {
  KEYBOARD=1,
  JOYSTICK=2
};

enum Gamemode {NO_GAMEMODE,SINGLE,COMPUTER,NOVICE,MULTIPLE};

enum {
  SOUND_PRIORITY_THEME         =5,
  SOUND_PRIORITY_EXPLOSION     =10,
  SOUND_PRIORITY_BOMB_FALLING  =20,
  SOUND_PRIORITY_FIRING        =30,
  SOUND_PRIORITY_PLANE_FALLING =40,
  SOUND_PRIORITY_PLANE_TROUBLED=50,
  SOUND_PRIORITY_PLANE_OK      =60
};

enum Drawcharmode {
  BLACK_BACKGROUND,
  TRANSPARENT_BACKGROUND,
  XOR_CHARACTER
};

typedef int ground_t;

enum {MAX_X=3000};
enum {RUNWAY_LENGTH=20};
enum {PLANE_WIDTH=16};
enum {PLANE_HEIGHT=16};
enum {ANGLES=16};
enum {MAX_AMMO=200};
enum {MAX_BOMBS=5};
enum {MAX_FUEL=3*MAX_X};
enum {MAX_Y=200};
enum {TARGET_WIDTH=16};
enum {TARGET_HEIGHT=16};

extern int screen_width;
extern int screen_height;
extern int units_per_screen_width;
extern int units_per_screen_height;
enum {WINDOW_BORDER=18};
namespace {
  inline int screen_center_x() {return (screen_width-PLANE_WIDTH)/2;}
  inline int screen_center_y() {return (screen_height+PLANE_HEIGHT)/2;}
  inline int mapscalex() {return MAX_X*2/screen_width+1;}
  inline int mapscaley() {return MAX_Y/(WINDOW_BORDER-2)+1;}
  inline int planesgaugex() {return screen_center_x()-25;}
  inline int fuelgaugex() {return screen_center_x()-20;}
  inline int bombgaugex() {return screen_center_x()-15;}
  inline int ammogaugex() {return screen_center_x()-10;}
  inline int zoomx() {return screen_width/units_per_screen_width;}
  inline int zoomy() {return screen_height/units_per_screen_height;}
}

namespace {
  template<class T> inline std::string tostring(T t)
  {
    std::stringstream s;
    s << t;
    return s.str();
  }
}

enum {GAUGE_HEIGHT=10};
enum {FLOCKLIFE=5};
enum {FLOCK_WIDTH=16};
enum {FLOCK_HEIGHT=16};
enum {MIN_FLOCK_X=370};
enum {MAX_FLOCK_X=MAX_X-MIN_FLOCK_X};
enum {BIRDLIFE=4};
enum {BIRD_WIDTH=4};
enum {BIRD_HEIGHT=2};
enum {OX_WIDTH=16};
enum {OX_HEIGHT=16};
enum {MAX_SPEED=8};
enum {MIN_SPEED=4};

enum {
  KEY_ACCEL    =1,
  KEY_BRAKE    =2,
  KEY_CLIMB    =4,
  KEY_DESCEND  =8,
  KEY_FLIP     =0x10,
  KEY_FIRE     =0x20,
  KEY_BOMB     =0x100,
  KEY_GOHOME   =0x200,
  KEY_SOUND    =0x400,
  KEY_BREAK    =0x800,
  KEY_PAUSEGAME=0x4000,
  KEY_DISCONN  =0x8000
};

namespace {
  template <class D,class B> inline bool instanceof(const B& b)
  {
    return dynamic_cast<const D*>(&b)!=0;
  }
}

enum Gamestatus {PLAYING,RESTARTING,EXITING};
enum {BULLETSPEED=10};
enum {BULLETLIFE=10};
enum {QUIT=-5000};
enum {MAX_THROTTLE=4};
enum {HOMEDIST=16};
/*enum {LONGWAY=200};*/
enum {CRUISEHEIGHT=MAX_Y-50};

enum {
  BULLET_MISS,
  BULLET_SHORTRANGE,
  BULLET_LONGRANGE
};

/*enum {NEAR=125*125};*/
enum {CLOSE=32};
enum {TERMINAL_VELOCITY=-10};
enum {FALLCOUNT=10};
enum {STALLCOUNT=6};
enum {ENDTIME=18};
enum {SCORE_PLANE=50};
enum {BOMB_DELAY=10};
enum {BOMBLIFE=5};
enum {BOMB_WIDTH=8};
enum {BOMB_HEIGHT=8};
enum {SMOKE_WIDTH=1};
enum {SMOKE_HEIGHT=1};
enum {SMOKELIFE=10};
enum {FRAGLIFE=3};
enum {FRAG_WIDTH=8};
enum {FRAG_HEIGHT=8};
enum {TARGETHITCOUNT=10};
enum {MAXCRCOUNT=10};
enum {BULLET_WIDTH=1};
enum {BULLET_HEIGHT=1};
enum {SAFERESET=32};

enum Spritetype {
  BOMB,
  TARGET,
  DEBRIS,
  FRAG,
  PLANE,
  PLANE_FINALE,
  PLANE_FALLING,
  FLOCK,
  BIRD,
  OX,
  PIXEL
};

enum Soundevent {
  BOMB_DESTROYED,
  TARGET_DESTROYED,
  TARGET_FIRED,
  SOUND_EVENT_SIZE
};

enum Musicmode {
  NO_MUSIC,
  PLAY_NOTE,
  PLAY_SONG
};

enum Connmode {NO_CONN,SERVER,CLIENT};

namespace {
  const long clockTickRate=1193182L;
  const long clockTicksPerTicks=0x10000;
}

#endif /* SOPWITH_DEF_H */
