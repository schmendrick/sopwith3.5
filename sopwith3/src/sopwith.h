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
#ifndef SOPWITH_SOPWITH_H
#define SOPWITH_SOPWITH_H

#include "def.h"
#include <list>
#ifdef DEBUG
#include <fstream>
#endif
#include <vector>
#include <string>
#include <iostream>
#include "object.h"
#include "plane.h"

extern int version;
extern bool soundflag;
extern ground_t ground[MAX_X];
extern bool ibmkeyboard;
extern int controls;
extern int level;
extern int framecounter;
extern int targetrange2;
extern Gamestatus gamestatus;
extern int latencycount;
extern int minspeed;
extern int maxspeed;
extern Gamemode gamemode;
extern std::list<Object*> objectlist;
extern short randv;
extern int collxadj;
extern int collyadj;
extern bool forceredraw;
extern bool complementsprites;
extern Connmode connmode;
extern int lastkey;
extern bool exiting;
extern int latency;
extern int requested_screen_width;
extern Plane* headplanes[2];
extern std::string address;
extern int computerplayers;
extern const int maxplanes;
extern int playerindex;
extern int remoteplanes;
extern bool fixedorder;
extern int maxremoteplanes;
extern unsigned int speedtick;
#ifdef DEBUG
extern std::ofstream errorfile;
#endif

int start(int argc,char* argv[]);
void getoptions(const std::vector<std::string>& argv);
void getoption(const std::string& s,int& option);
void getoption(const std::string& s,std::string& option);
template<class T> void getoption(const std::string& s,T& option,T setting);
void run();
void inithistory();
short getshort(std::istream& in);
std::ostream& putshort(std::ostream& out,short s);
std::istream& putbackshort(std::istream& in,short s);
void flushhistory();
void titlefield();
Gamemode getgamemode();
void processtimerticks();
void getcontrol();
Connmode getside();
void initlevel();
void copyground();
void deepclear(std::list<Object*>& objectlist);
void add(std::list<Object*>& objectlist,Object* obj);
void initdisplay();
void drawmapground();
void initscores();
void displayscore(Plane* p);
void displayplanesgauge(Plane* plane);
void displaygauge(int x,int h,int hmax,int colour);
void displayfuelgauge(Plane* plane);
void displaybombgauge(Plane* plane);
void displayammogauge(Plane* plane);
void initdifficulty();
void mainloop();
void updateobjects();
inline int screenleft();
bool onscreen(Object* obj);
void updatescreen();
void checkcollisions();
inline bool moretotheleft(Object* obj1,Object* obj2);
bool collided(Object* obj1,Object* obj2);
bool crashed(Object* obj);
void restart();
int history(int keys);
void loser(Plane* plane);
void winner(Plane* plane);
void endgame(int targetcolour);
#ifdef DEBUG
std::string stripped(const std::string& s);
#endif

#endif /* SOPWITH_SOPWITH_H */
