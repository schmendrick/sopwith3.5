/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2001 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

#ifndef _SOPWITH_DEF_H
#define _SOPWITH_DEF_H

/* General stuff */

#ifndef VERSION    /* So we can define it as a compiler parameter */
#define VERSION 7  /* Or 1, or 7 for 7.F15 (DE) */
#endif

typedef int bool;
typedef int ground_t;

enum {TRUE=-1,FALSE};

#define ANGLES 16
#define fcos(r,a) ((r)*sine[((a)+ANGLES/4)&(ANGLES-1)])
#define fsin(r,a) ((r)*sine[(a)&(ANGLES-1)])

#define MAX_OBJECTS 100
#define SOUNDSIZE 100

#if VERSION==2
#define SPEED 20
#else
#define SPEED 15
#endif

#if VERSION==1
#define TEMPO 1080
#else
#define TEMPO 1440
#endif


/* Hardware */

#define SCR_SEG 0xb800
#define SCR_INTERLACE 0x2000
#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#define SCR_BYTES 80

enum {
  PORT_TIMER0=0x40,
  PORT_TIMER2=0x42,
  PORT_TIMERC=0x43,
  PORT_KEYB  =0x60,
  PORT_SPKR  =0x61,
  PORT_JOY   =0x201
};

enum {
  JOY_X   =1,
  JOY_Y   =2,
  JOY_BUT1=0x10,
  JOY_BUT2=0x20
};

enum {
  JOYLOW =0x280,
  JOYHIGH=0x780,
  JOYMAX =0xa00
};

enum {
  BIOS_TEXT    =3,
  BIOS_GRAPHICS=4,
};

enum {
  SC_P    =0x19,
  SC_S    =0x1f,
  SC_H    =0x23,
  SC_Z    =0x2c,
  SC_X    =0x2d,
  SC_C    =0x2e,
  SC_V    =0x2f,
  SC_B    =0x30,
  SC_COMMA=0x33,
  SC_DOT  =0x34,
  SC_SLASH=0x35,
  SC_SPACE=0x39,
  SC_BREAK=0x46
};

enum {
  MOV_BLACK,
  MOV_CYAN,
  MOV_MAGENTA,
  MOV_WHITE,
  MOV_GREEN=3, /* Was 7 */
  MOV_TAN=1,   /* Was 9 */
  MOV_BROWN=3, /* Was 11 */
  XOR_BLACK=0x80,
  XOR_CYAN,
  XOR_MAGENTA,
  XOR_WHITE
};

#define MAGENTA4 (MOV_MAGENTA*0x55)


/* Game parameters: Numbers of things */

#define MAXLIVES 5
#define MAX_AMMO 200
#define MAX_BOMBS 5
#define MAX_FLOCKS 4
#define MAX_MISSILES 5
#define MAX_OXEN 2
#define MAX_PLAYERS 4
#define MAX_STARBURSTS  5
#define MAX_TARGETS 20
#define STRAYBIRDS 1
#if VERSION==2
#define MAX_AVOIDABLES MAX_TARGETS
#else
#define MAX_AVOIDABLES (MAX_TARGETS+MAX_OXEN)
#endif

#define TARGETHITCOUNT 10              /* Target hit count before exploding */


/* Game parameters: Temporal */

#define BIRDLIFE 4
#define BOMBLIFE 5
#define BOMB_DELAY 10
#define BULLETLIFE 10
#define ENDTIME 18
#define FALLCOUNT 10
#define FLOCKLIFE 5
#define FRAGLIFE 3
#define MAXCRCOUNT 10              /* Number of turns as crashed */
#define MAX_FUEL (3*MAX_X)
#define MISSILELIFE 50
#define SMOKELIFE 10
#define STALLCOUNT 6
#define STARBURSTLIFE 20


/* Game parameters: Speeds */

#define BULLETSPEED 10
#define MAX_SPEED 8
#define MAX_THROTTLE 4
#define MIN_SPEED 4
#define TERMINAL_VELOCITY (-10)


/* Screen layout: Horizontal */

#define PLANE_WIDTH 16
#define RUNWAY_LENGTH 20
#if VERSION==2
#define PLANESGAUGEX    (SCR_CENTER-25)
#define FUELGAUGEX      (SCR_CENTER-20)
#define BOMBGAUGEX      (SCR_CENTER-15)
#define AMMOGAUGEX      (SCR_CENTER-10)
#else
#define PLANESGAUGEX    (SCR_CENTER-25)
#define FUELGAUGEX      (SCR_CENTER-22)
#define BOMBGAUGEX      (SCR_CENTER-19)
#define AMMOGAUGEX      (SCR_CENTER-16)
#endif
#define GHOSTX          (SCR_CENTER-21)
#define MISSILEGAUGEX   (SCR_CENTER-13)
#define STARBURSTGAUGEX (SCR_CENTER-10)

#define MAX_X 3000
#define MIN_FLOCK_X (SCR_WIDTH+50)
#define MAX_FLOCK_X (MAX_X-MIN_FLOCK_X)
#if VERSION==2
#define LONGWAY 200
#else
#define LONGWAY 160
#endif

#define MAPDIVX (MAX_X/SCR_WIDTH*2+1)


/* Screen layout: Vertical */

#define MAX_Y 200
#define CRUISEHEIGHT (MAX_Y-50)
#define GAUGE_HEIGHT 10
#define PLANE_HEIGHT 16
#define SCROLL_LIMIT 180
#define SCR_CENTER 152
#define SCR_SCROLLBOT 16

#define MAPDIVY (MAX_Y/SCR_SCROLLBOT+1)


/* Radial distances */

#define HOMEDIST 16
#define SAFERESET 32
#define CLOSE 32
#if VERSION==1
#define NEAR (75*75)
#elif VERSION==2
#define NEAR (125*125)
#else
#define NEAR (150*150)
#endif


/* Meaningless enumerations */

enum {
  KEY_ACCEL    =1,
  KEY_BRAKE    =2,
  KEY_CLIMB    =4,
  KEY_DESCEND  =8,
  KEY_FLIP     =0x10,
  KEY_FIRE     =0x20,
  KEY_BOMB     =0x100,
  KEY_HOME     =0x200,
  KEY_SOUND    =0x400,
  KEY_BREAK    =0x800,
  KEY_MISSILE  =0x1000,
  KEY_STARBURST=0x2000,
  KEY_PAUSE    =0x4000
};

enum {
  TARGET_FLAG,
  TARGET_CHIMNEY,
  TARGET_FUEL,
  TARGET_TANK
};

enum {
  SINGLE,
  MULTIPLE,
  COMPUTER,
  ASYNCH,
  NOVICE
};

enum { /* Object types */
  GROUND,
  PLANE,
  BOMB,
  SHOT,
  TARGET,
  EXPLOSION,
  SMOKE,
  FLOCK,
  BIRD,
  OX,
  MISSILE,
  STARBURST,
  DUMMYTYPE
};

enum { /* It's very silly to give names to most of these */
  FRAG_PERSON,
  FRAG_WHEEL,
  FRAG_BRICK,
  FRAG_ANVIL,
  FRAG_FROG,
  FRAG_CHUNKS1,
  FRAG_CHUNKS2,
  FRAG_DUST
};

enum {
  WAITING,
  FLYING,
  CRASHED=4,
  FALLING,
  STANDING, /* Used for buildings and oxen */
  STALLED,
  WOUNDED,
  WOUNDSTALL,
  FINISHED=91, /* Was 3 in Sopwith 1 */
  GHOST,
  GHOSTCRASHED,
  GHOSTSTALLED
};

enum {
  NOTFINISHED,
  WINNER,
  LOSER
};

enum {
  SOUND_OFF      =0,
  SOUND_TITLE    =5,
  SOUND_EXPLOSION=10,
  SOUND_BOMB     =20,
  SOUND_FIRING   =30,
  SOUND_FALLING  =40,
  SOUND_HIT      =50,
  SOUND_PLANE    =60,
  SOUND_NONE     =0x7fff
};

enum {
  BULLET_MISS,
  BULLET_SHORTRANGE,
  BULLET_LONGRANGE
};

#define QUIT (-5000)

#endif /* _SOPWITH_DEF_H */
