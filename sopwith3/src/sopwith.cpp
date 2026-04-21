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
#ifndef SOPWITH_SOPWITH_CPP
#define SOPWITH_SOPWITH_CPP

#include "sopwith.h"
#include "init.h"
#include "resource.h"
#include "sw_excep.h"
#include "timer.h"
#include "graphics.h"
#include "sound.h"
#include "keyboard.h"
#include "joystick.h"
#include "network.h"
#include "message.h"
#include "soundsys.h"
#include "ground.h"
#include "object.h"
#include "player.h"
#include "computer.h"
#include "remote.h"
#include "target.h"
#include "flock.h"
#include "ox.h"
#include "bomb.h"
#include "frag.h"
#include "smoke.h"
#include "bullet.h"
#include "replay_writer.h"
#include "replay_visual_validation.h"
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <list>
#include <cstring>
#include <algorithm>
#include <utility>
#include <map>
#include <chrono>

int version=2;
const unsigned int singleplayermaxlives=5;
#ifdef DEBUG
unsigned int speed=0;
std::ofstream errorfile("error");
#else
#ifdef DEBUG2
unsigned int speed=0;
#else
unsigned int speed;
#endif
#endif

/* options */
int latency=1;
bool soundflag=true;
int controls=0;
bool ibmkeyboard=false;
int level=0;
std::string playbackfilename;
std::string recordfilename;
/* options */

Gamemode gamemode;
short randv;
std::ifstream inputfile;
std::ofstream outputfile;
unsigned long processedtimerticks=0;
unsigned int speedtick=0;
ground_t ground[MAX_X];
int savescore;
int minspeed;
int maxspeed;
int targetrange2;
Gamestatus gamestatus;
int latencycount;
int framecounter=0;
bool forceredraw;
int collxadj;
int collyadj;
int historykeys=0;
std::list<Object*> objectlist;
bool gameover=false;
int units_per_screen_width=320;
int units_per_screen_height=200;
bool complementsprites=false;
Connmode connmode=NO_CONN;
int lastkey;
bool exiting=false;
Plane* headplanes[2]={0,0};
std::string address;
bool showhelp=false;
int computerplayers=0;
const int maxplanes=8;
int playerindex;
int remoteplanes=0;
bool fixedorder=false;
int maxremoteplanes=maxplanes-1;

int start(int argc,char* argv[])
{
  try {
    Resource initobject(init,deinit);
    try {
      try {
        getoptions(std::vector<std::string>(argv+1,argv+argc));
        /* -i sets ibmkeyboard only; -k also sets controls. Without this, controls==0 forces
           getcontrol() even when gamemode was set on the command line (e.g. -s -i). */
        if (ibmkeyboard && controls == 0 && gamemode != NO_GAMEMODE)
          controls = KEYBOARD;
      }
      catch (std::exception& e) {
        std::stringstream exception;
        exception << e.what();
        message(exception.str());
        showhelp=true;
      }
      if (showhelp) {
        std::string helptext;
        helptext+="SOPWITH 3.5, Copyright (C) 2026, Felix Schmutz\n";
        helptext+="Based on SOPWITH 3, Copyright (C) 2001 The Sopwith Team\n";
        helptext+="Based on SOPWITH, Copyright (C) 1984-2000 David L. Clark\n";
        helptext+="Check out the Sopwith 3.5 GitHub repository at:\n";
        helptext+="https://github.com/felixschmutz/sopwith3.5\n";
        helptext+="And make sure to check out the original Sopwith website at:\n";
        helptext+="http://sopwith3.sourceforge.net/\n";
        helptext+="Executable built on " __DATE__ " at " __TIME__ "\n";
        helptext+="Usage: sopwith3 [options]\n";
        helptext+="The (CASE SENSITIVE) options are:\n";
        helptext+="-s : Single player (expert)           -c : Single player against computer\n";
        helptext+="-n : Single player (novice)";
        if (networkavailable)
          helptext+="           -m : Multiple players on network\n";
        else
          helptext+='\n';
        helptext+="-k : Keyboard only                    -j : Joystick and keyboard\n";
        helptext+="-i : IBM PC Keyboard                  -q : Begin game with sound off\n";
        helptext+="-y#: Set latency to # (default:1) (!) -g#: Start on level # (default:0) (!)\n";
        helptext+="-h*: Record game to binary file *      -v*: Play back binary file *\n";
        helptext+="-D : Turn on certain \"The Author's Edition\" features (!)\n";
        helptext+="-F : Generate missing graphics files from built-in sprites (if possible)\n";
        helptext+="-S : Play as server                   -C : Play as client\n";
        helptext+="-A*: Connect to server with address * (if possible)\n";
        helptext+="-R#: Set maximum amount of clients to #\n";
        helptext+="-E#: Set amount of computer planes to # (may be truncated) (!)\n";
        helptext+="-O : Position players in order of connection (!)\n";
        helptext+="-H : Show this help\n";
        helptext+="-h/-v require attached filename (no space), e.g. -hmy.rec -vmy.rec\n";
        helptext+="-Recording also writes <file>.state.txt (current scaffold)\n";
        helptext+="-h and -v are single player only      -E is multiplayer only\n";
        helptext+="(!) will be overridden by server";
        message(helptext);
      }
      else
        run();
    }
    catch (std::exception& e) {
      std::stringstream exception;
      exception << "Error: " << e.what();
      message(exception.str());
      return EXIT_FAILURE;
    }
    catch(...) {
      message(std::string("Unknown exception"));
      return EXIT_FAILURE;
    }
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "Unknown exception" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void getoptions(const std::vector<std::string>& argv)
{
  replay_visual_log_note("options_parse_start");
  const std::vector<std::string>::const_iterator end=argv.end();
  for (std::vector<std::string>::const_iterator i=argv.begin();i!=end;++i) {
    if (i->length()<2 || (*i)[0]!='-') {
      std::stringstream exception;
      exception << "Unrecognized option: " << *i;
      throw sw_excep(exception);
    }
    else
      switch ((*i)[1]) {
        case 's': getoption(*i,gamemode,SINGLE); break;
        case 'c': getoption(*i,gamemode,COMPUTER); break;
        /**/case 'n': getoption(*i,gamemode,NOVICE); break;/**/
        case 'm':
          if (networkavailable)
            getoption(*i,gamemode,MULTIPLE);
          break;
        case 'k': getoption(*i,controls,(controls|KEYBOARD)&~JOYSTICK); break;
        case 'j': getoption(*i,controls,controls|JOYSTICK); break;
        case 'i': getoption(*i,ibmkeyboard,true); break;
        case 'q': getoption(*i,soundflag,false); break;
        case 'y': getoption(*i,latency); break;
        case 'g': getoption(*i,level); break;
        case 'h':
          getoption(*i,recordfilename);
          replay_visual_log_note(std::string("record_option=")+recordfilename);
          break;
        case 'v':
          getoption(*i,playbackfilename);
          replay_visual_log_note(std::string("playback_option=")+playbackfilename);
          break;
        /**/case 'D': getoption(*i,version,7); break;/**/
        /**/case 'F': getoption(*i,complementsprites,true); break;/**/
        case 'S': getoption(*i,connmode,SERVER); break;
        case 'C': getoption(*i,connmode,CLIENT); break;
        case 'A': getoption(*i,address); break;
        case 'R': getoption(*i,maxremoteplanes); break;
        case 'E': getoption(*i,computerplayers); break;
        case 'O': getoption(*i,fixedorder,true); break;
        case 'H': getoption(*i,showhelp,true); break;
        default:
          {
            std::stringstream exception;
            exception << "Unrecognized option: " << *i;
            throw sw_excep(exception);
          }
        break;
    }
  }
  replay_visual_log_note("options_parse_done");
}

void getoption(const std::string& s,int& option)
{
  std::stringstream stream(s.substr(2));

  stream >> option;
  if (!stream /*|| !stream.eof()*/) {
    std::stringstream exception;
    exception << "Option " << s.substr(0,2) << " requires a valid integer";
    if (!s.substr(2).empty())
      exception << ", not: " << s.substr(2);
    throw sw_excep(exception);
  }
}

void getoption(const std::string& s,std::string& option)
{
  if ((s.substr(2)).empty()) {
    std::stringstream exception;
    exception << "Option " << s.substr(0,2) << " requires a non-empty string";
    throw sw_excep(exception);
  }
  option=s.substr(2);
}

template<class T> void getoption(const std::string& s,T& option,T setting)
{
  if (!(s.substr(2)).empty()) {
    std::stringstream warning;
    warning << "Ignoring extra information for option " << s.substr(0,2) << ": " <<
               s.substr(2);
    message(warning.str());
  }
  option=setting;
}

namespace {

bool menu_key_exits(int c)
{
  return c=='\x1b' || c==27;
}

void request_app_exit()
{
  exiting=true;
  gamestatus=EXITING;
}

void debug_log(const char* runId,const char* hypothesisId,const char* location,const char* message,const std::string& data)
{
  std::ofstream dbg("F:/Development/ai/sopwith3/sopwith3/debug-bea600.log",std::ios::app);
  if (!dbg)
    return;
  const long long ts=std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  dbg << "{\"sessionId\":\"bea600\",\"runId\":\"" << runId
      << "\",\"hypothesisId\":\"" << hypothesisId
      << "\",\"location\":\"" << location
      << "\",\"message\":\"" << message
      << "\",\"data\":{" << data
      << "},\"timestamp\":" << ts << "}\n";
}

bool run_front_end_menus()
{
  if (gamemode==NO_GAMEMODE && controls==0) {
    displaytitlescreen();
    setsound(SOUND_PRIORITY_THEME,0,0);
    updatesound();
    while (!exiting) {
      processtimerticks();
      int c=inkey();
      if (c!=0) {
        // #region agent log
        debug_log("baseline","H8","sopwith.cpp:run_front_end_menus","menu_key_detected",
                  std::string("\"timer\":")+tostring(static_cast<int>(processedtimerticks))+
                  ",\"key\":"+tostring(c));
        // #endregion
      }
      static unsigned long lastMenuLogTick=static_cast<unsigned long>(-1);
      if ((processedtimerticks%120)==0 && processedtimerticks!=lastMenuLogTick) {
        // #region agent log
        debug_log("baseline","H3","sopwith.cpp:run_front_end_menus","menu_loop_tick",
                  std::string("\"timer\":")+tostring(static_cast<int>(processedtimerticks))+
                  ",\"key\":"+tostring(c));
        // #endregion
        lastMenuLogTick=processedtimerticks;
      }
      if (c==0)
        continue;
      if (menu_key_exits(c))
        request_app_exit();
      break;
    }
    clearsounds();
    cleartitlescreen();
  }
  if (exiting)
    return false;

  if (gamemode==NO_GAMEMODE)
    gamemode=getgamemode();
  if (exiting)
    return false;

  if (controls==0)
    getcontrol();
  if (exiting)
    return false;

  if (gamemode==MULTIPLE && connmode==NO_CONN)
    connmode=getside();

  return !exiting;
}

}  // namespace

void run()
{
  #ifndef DEBUG
  #ifndef DEBUG2
  if (version==2)
    speed=20;
  else
    speed=15;
  #endif
  #endif
  randv=time(0);
  srand(randv);
  Resource historyobject(inithistory,flushhistory);
  /*initbreak();*/
  Resource graphicsobject(graphicsmode,textmode);
  Resource keyboardobject(initkeyboard,deinitkeyboard);
  Resource soundobject(initsound,deinitsound);
  Resource timerobject(inittimer,deinittimer);
  // #region agent log
  debug_log("baseline","H1","sopwith.cpp:run","post_init","\"timer\":0");
  // #endregion
  if (!run_front_end_menus()) {
    // #region agent log
    debug_log("baseline","H15","sopwith.cpp:run","menu_exit_early",
              std::string("\"gamestatus\":")+tostring(static_cast<int>(gamestatus))+
              ",\"exiting\":"+(exiting?"1":"0"));
    // #endregion
    return;
  }
  Resource joystickobject;
  if ((controls&JOYSTICK)!=0) {
    initjoystick();
    joystickobject.destruction=deinitjoystick;
  }
  Resource networkobject;
  if (gamemode==MULTIPLE) {
    initnetwork();
    networkobject.destruction=deinitnetwork;
  }
  initlevel();
  mainloop();
  // #region agent log
  debug_log("baseline","H15","sopwith.cpp:run","after_mainloop",
            std::string("\"gamestatus\":")+tostring(static_cast<int>(gamestatus))+
            ",\"exiting\":"+(exiting?"1":"0"));
  // #endregion
}

void inithistory()
{
  inputfile.exceptions(~std::ios::goodbit);
  outputfile.exceptions(~std::ios::goodbit);
  /**/short historybufsize=static_cast<short>(-1);/**/ /* -1 is arbitrary */
  if (!playbackfilename.empty()) {
    replay_visual_log_note(std::string("playback_open_attempt=")+playbackfilename);
    try {
      inputfile.open(playbackfilename.c_str(),std::ios::binary);
    }
    catch(...) {
      throw sw_excep("Unable to open history input file");
    }
    randv=getshort(inputfile);
    historybufsize=getshort(inputfile);
    replay_visual_log_event("playback_open", true);
    replay_visual_log_note(std::string("playback_header_seed=")+tostring(randv));
  }
  if (!recordfilename.empty()) {
    try {
      outputfile.open(recordfilename.c_str(),std::ios::binary);
    }
    catch(...) {
      throw sw_excep("Unable to open history output file");
    }
    putshort(outputfile,randv);
    putshort(outputfile,historybufsize);
    std::string statefile = recordfilename + ".state.txt";
    if (replay_open_writer(statefile)) {
      replay_write_session_row(1, randv, latency, playerindex);
    }
  }
}

short getshort(std::istream& in)
{
  return in.get()+(in.get()<<8); /* Little endian */
}

std::ostream& putshort(std::ostream& out,short s)
{
  out.put(s&0xff).put(s>>8); /* Little endian */
  return out;
}

std::istream& putbackshort(std::istream& in,short s)
{
  in.putback(s>>8);   /* Little */
  in.putback(s&0xff); /* endian */
  return in;
}

void flushhistory()
{
  replay_visual_log_event("playback_close", true);
  replay_close_writer();
  if (!recordfilename.empty())
    outputfile.close();
  if (!playbackfilename.empty())
    inputfile.close();
}

void titlefield()
{
  clearscreen();
  initscreen();
  drawstr(13*8*zoomx(), 8*8*zoomy(),colour_white,BLACK_BACKGROUND,"S O P W I T H");
  drawstr(12*8*zoomx(),11*8*zoomy(),colour_cyan, BLACK_BACKGROUND,"BMB");
  drawstr(16*8*zoomx(),11*8*zoomy(),colour_white,BLACK_BACKGROUND,"Compuscience");
  displayscreen();
}

Gamemode getgamemode()
{
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Key: S - single player");
  drawstr(0*8*zoomx(),21*8*zoomy(),colour_white,BLACK_BACKGROUND,"     C - single player against computer");
  if (networkavailable)
    drawstr(0*8*zoomx(),22*8*zoomy(),colour_white,BLACK_BACKGROUND,"     M - multiple players on network");
  displayscreen();
  while (!exiting) {
    processtimerticks();
    int c=inkey();
    if (c!=0) {
      // #region agent log
      debug_log("baseline","H8","sopwith.cpp:getgamemode","gamemode_key",
                std::string("\"key\":")+tostring(c));
      // #endregion
    }
    if (menu_key_exits(c)) {
      request_app_exit();
      return NO_GAMEMODE;
    }
    switch (toupper(c)) {
      case 'S':
        return SINGLE;
      case 'C':
        return COMPUTER;
      case 'M':
        if (networkavailable)
          return MULTIPLE;
    }
  }
  return NO_GAMEMODE;
}

void processtimerticks()
{
  unsigned long now=timer();
  if (processedtimerticks==now) {
    static unsigned long idleloops=0;
    static unsigned long idleyields=0;
    ++idleloops;
    timeridle();
    ++idleyields;
    if ((idleyields%120)==0) {
      // #region agent log
      debug_log("baseline","H14","sopwith.cpp:processtimerticks","idle_yield",
                std::string("\"processed\":")+tostring(static_cast<int>(processedtimerticks))+
                ",\"now\":"+tostring(static_cast<int>(now))+
                ",\"idleyields\":"+tostring(static_cast<int>(idleyields)));
      // #endregion
    }
    if ((idleloops%100000000)==0) {
      // #region agent log
      debug_log("baseline","H1","sopwith.cpp:processtimerticks","timer_stalled",
                std::string("\"processed\":")+tostring(static_cast<int>(processedtimerticks))+
                ",\"now\":"+tostring(static_cast<int>(now))+
                ",\"idleloops\":"+tostring(static_cast<int>(idleloops)));
      // #endregion
    }
    return;
  }
  static long long firstAdvanceMs=0;
  static unsigned long advancedTicks=0;
  if (firstAdvanceMs==0)
    firstAdvanceMs=std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  advancedTicks+=now-processedtimerticks;
  // #region agent log
  debug_log("baseline","H1","sopwith.cpp:processtimerticks","timer_advanced",
            std::string("\"processed\":")+tostring(static_cast<int>(processedtimerticks))+
            ",\"now\":"+tostring(static_cast<int>(now)));
  // #endregion
  if ((advancedTicks%120)==0) {
    const long long curMs=std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
    // #region agent log
    debug_log("baseline","H6","sopwith.cpp:processtimerticks","timer_rate_sample",
              std::string("\"advancedTicks\":")+tostring(static_cast<int>(advancedTicks))+
              ",\"elapsedMs\":"+tostring(static_cast<int>(curMs-firstAdvanceMs)));
    // #endregion
  }
  for (;processedtimerticks!=now;++processedtimerticks) {
    speedtick+=10;
    soundadjust();
  }
}

void getcontrol()
{
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Key: 1 - Joystick with IBM Keyboard");
  drawstr(0*8*zoomx(),21*8*zoomy(),colour_white,BLACK_BACKGROUND,"     2 - Joystick with non-IBM Keyboard");
  drawstr(0*8*zoomx(),22*8*zoomy(),colour_white,BLACK_BACKGROUND,"     3 - IBM Keyboard only");
  drawstr(0*8*zoomx(),23*8*zoomy(),colour_white,BLACK_BACKGROUND,"     4 - Non-IBM keyboard only");
  displayscreen();
  while (!exiting) {
    processtimerticks();
    int c=inkey();
    if (c!=0) {
      // #region agent log
      debug_log("baseline","H8","sopwith.cpp:getcontrol","control_key",
                std::string("\"key\":")+tostring(c));
      // #endregion
    }
    if (menu_key_exits(c)) {
      request_app_exit();
      return;
    }
    switch (c) {
      case '1':
        controls=KEYBOARD|JOYSTICK;
        ibmkeyboard=true;
        return;
      case '2':
        controls=KEYBOARD|JOYSTICK;
        ibmkeyboard=false;
        return;
      case '3':
        controls=KEYBOARD;
        ibmkeyboard=true;
        return;
      case '4':
        controls=KEYBOARD;
        ibmkeyboard=false;
        return;
    }
  }
}

Connmode getside()
{
  titlefield();
  drawstr(0*8*zoomx(),20*8*zoomy(),colour_white,BLACK_BACKGROUND,"Key: S - Server (start game)");
  drawstr(0*8*zoomx(),21*8*zoomy(),colour_white,BLACK_BACKGROUND,"     C - Client (join game)");
  displayscreen();
  while (!exiting) {
    processtimerticks();
    int c=inkey();
    if (c!=0) {
      // #region agent log
      debug_log("baseline","H8","sopwith.cpp:getside","side_key",
                std::string("\"key\":")+tostring(c));
      // #endregion
    }
    if (menu_key_exits(c)) {
      request_app_exit();
      return NO_CONN;
    }
    switch (toupper(c)) {
      case 'S':
        return SERVER;
      case 'C':
        return CLIENT;
    }
  }
  return NO_CONN;
}

void initlevel()
{
  copyground();
  deepclear(objectlist);
  if (gamemode!=MULTIPLE)
    add(objectlist,player=new Playerplane(1270,colour_cyan,false,false,singleplayermaxlives,false,gamemode==NOVICE,gamemode==NOVICE));
  else {
    static int runwayx[maxplanes]={588,2456,1270,1720,1305,1685,1340,1650};
    static int runwayleft[maxplanes]={false,true,false,true,false,true,false,true};
    static int colour[maxplanes]={colour_cyan,colour_magenta,colour_cyan,colour_magenta,colour_cyan,colour_magenta,colour_cyan,colour_magenta};
    int i=0;
    for (;i<playerindex;++i)
      add(objectlist,new Remoteplane(runwayx[i],colour[i],runwayleft[i],true,1,false,false,false,i));
    add(objectlist,player=new Playerplane(runwayx[i],colour[i],runwayleft[i],true,1,false,false,false));
    ++i;
    for (;i<=remoteplanes;++i)
      add(objectlist,new Remoteplane(runwayx[i],colour[i],runwayleft[i],true,1,false,false,false,i));
    for (int j=0;i<maxplanes&&j<computerplayers;++i,++j)
      add(objectlist,new Computerplane(runwayx[i],colour[i],runwayleft[i],true,-1,-1,true,true,true));
  }
  if (gamemode==COMPUTER) {
    add(objectlist,new Computerplane(1720,colour_magenta,true,false,1155,2088,true,true,true));
    add(objectlist,new Computerplane( 588,colour_magenta,false,false,0,1154,true,true,true));
    add(objectlist,new Computerplane(2456,colour_magenta,true,false,2089,MAX_X,true,true,true));
  }
  headplanes[0]=0;
  headplanes[1]=0;
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    Plane* plane=dynamic_cast<Plane*>(*obj);
    if (plane!=0) {
      if (plane->colour==colour_cyan && headplanes[0]==0) {
        headplanes[0]=plane;
        if (gamemode!=MULTIPLE || headplanes[1]!=0)
          break;
      }
      else
        if (gamemode==MULTIPLE && plane->colour==colour_magenta && headplanes[1]==0) {
          headplanes[1]=plane;
          if (headplanes[0]!=0)
            break;
        }
    }
  }
  int colour_targets=(gamemode==MULTIPLE ? colour_cyan : colour_magenta);
  add(objectlist,new Target( 191,colour_targets,Target::CHIMNEY));
  add(objectlist,new Target( 284,colour_targets,Target::TANK));
  add(objectlist,new Target( 409,colour_targets,Target::CHIMNEY));
  add(objectlist,new Target( 539,colour_targets,Target::CHIMNEY));
  add(objectlist,new Target( 685,colour_targets,Target::TANK));
  add(objectlist,new Target( 807,colour_targets,Target::FLAG));
  add(objectlist,new Target( 934,colour_targets,Target::CHIMNEY));
  add(objectlist,new Target(1210,colour_cyan   ,Target::FUEL));
  add(objectlist,new Target(1240,colour_cyan   ,Target::FLAG));
  add(objectlist,new Target(1440,colour_cyan   ,Target::TANK));
  add(objectlist,new Target(1550,colour_magenta,Target::TANK));
  add(objectlist,new Target(1750,colour_magenta,Target::FLAG));
  add(objectlist,new Target(1780,colour_magenta,Target::FUEL));
  add(objectlist,new Target(2024,colour_magenta,Target::CHIMNEY));
  add(objectlist,new Target(2159,colour_magenta,Target::CHIMNEY));
  add(objectlist,new Target(2279,colour_magenta,Target::TANK));
  add(objectlist,new Target(2390,colour_magenta,Target::TANK));
  add(objectlist,new Target(2549,colour_magenta,Target::FLAG));
  add(objectlist,new Target(2678,colour_magenta,Target::FLAG));
  add(objectlist,new Target(2763,colour_magenta,Target::CHIMNEY));
  initdisplay();
  if (gamemode!=NOVICE) {
    add(objectlist,new Flock(MIN_FLOCK_X,MAX_Y-1,2));
    add(objectlist,new Flock(MIN_FLOCK_X+1000,MAX_Y-1,2));
    add(objectlist,new Flock(MAX_FLOCK_X-1000,MAX_Y-1,-2));
    add(objectlist,new Flock(MAX_FLOCK_X,MAX_Y-1,-2));
    add(objectlist,new Ox(1376,80));
    add(objectlist,new Ox(1608,91));
  }
  initdifficulty();
  gamestatus=PLAYING;
}

void copyground()
{
  memcpy(ground,originalground,sizeof(ground_t)*MAX_X);
}

void deepclear(std::list<Object*>& objectlist)
{
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    delete *obj;
  }
  objectlist.clear();
}

void add(std::list<Object*>& objectlist,Object* obj)
{
  objectlist.push_back(obj);
  objectlist.splice(objectlist.end(),obj->childlist);
}

void initdisplay()
{
  clearscreen();
  initscores();
}

void drawmapground()
{
  int y=0,dx=0,maxy=0,sx=screen_center_x();
  for (int x=0;x<MAX_X;++x) {
    maxy=std::max(originalground[x],maxy);
    ++dx;
    if (dx==mapscalex()) {
      maxy/=mapscaley();
      if (maxy==y)
        putpixel(sx,maxy,colour_green);
      else
        if (maxy>y)
          for (++y;y<=maxy;++y)
            putpixel(sx,y,colour_green);
        else
          for (--y;y>=maxy;--y)
            putpixel(sx,y,colour_green);
      y=maxy;
      putpixel(sx,0,colour_brown);
      ++sx;
      dx=maxy=0;
    }
  }
  for (y=0;y<=MAX_Y/mapscaley();++y) {
    putpixel(screen_center_x(),y,colour_brown);
    putpixel(sx,y,colour_brown);
  }
  for (int x=0;x<screen_width;++x)
    putpixel(x,WINDOW_BORDER,colour_green);
}

void initscores()
{
  if (savescore!=0) {
    headplanes[0]->score=savescore;
    savescore=0;
  }
  displayscore(headplanes[0]);
  if (gamemode==MULTIPLE)
    displayscore(headplanes[1]);
}

void displayscore(Plane* p)
{
  std::string score=tostring(p->score);
  while (score.length()<6)
    score+=' ';
  drawstr(screen_width/20 + (p->colour==colour_cyan ? 0 : 7*8),screen_height-8,p->colour,BLACK_BACKGROUND,score.c_str());
}

void displayplanesgauge(Plane* plane)
{
  displaygauge(planesgaugex(),plane->lives,plane->maxlives,plane->colour);
}

void displaygauge(int x,int h,int hmax,int colour)
{
  int y;
  h=std::min(h*GAUGE_HEIGHT/hmax-1,GAUGE_HEIGHT-1);
  for (y=0;y<=h;++y)
    putpixel(x,y,colour);
  for (;y<GAUGE_HEIGHT;++y)
    putpixel(x,y,colour_black);
}

void displayfuelgauge(Plane* plane)
{
  displaygauge(fuelgaugex(),plane->life>>4,MAX_FUEL>>4,plane->colour);
}

void displaybombgauge(Plane* plane)
{
  displaygauge(bombgaugex(),plane->bombs,MAX_BOMBS,
               (plane->colour==colour_cyan ? colour_magenta : colour_cyan));
}

void displayammogauge(Plane* plane)
{
  displaygauge(ammogaugex(),plane->ammo,MAX_AMMO,colour_white);
}

void initdifficulty()
{
  minspeed=MIN_SPEED+level;
  maxspeed=MAX_SPEED+level;
  targetrange2=40+10*std::min(level,6);
  targetrange2*=targetrange2;
}

void mainloop()
{
  // #region agent log
  debug_log("baseline","H2","sopwith.cpp:mainloop","enter_mainloop",
            std::string("\"gamestatus\":")+tostring(static_cast<int>(gamestatus)));
  // #endregion
  processtimerticks();
  while (true) {
    // #region agent log
    debug_log("baseline","H9","sopwith.cpp:mainloop","outer_loop_state",
              std::string("\"gamestatus\":")+tostring(static_cast<int>(gamestatus)));
    // #endregion
    speedtick=0;
    while (gamestatus==PLAYING) {
      #ifdef DEBUG
      speed=0;
      #endif
      do {
        processtimerticks();
      } while (speedtick<speed);
      speedtick-=speed;
      updateobjects();
      updatejoy();
      updatescreen();
      updatejoy();
      checkcollisions();
      updatejoy();
      updatesound();
      if ((framecounter%60)==0) {
        // #region agent log
        debug_log("baseline","H2","sopwith.cpp:mainloop","playing_frame",
                  std::string("\"frame\":")+tostring(framecounter)+
                  ",\"speedtick\":"+tostring(static_cast<int>(speedtick)));
        // #endregion
      }
    }
    // #region agent log
    debug_log("baseline","H15","sopwith.cpp:mainloop","playing_loop_exit",
              std::string("\"gamestatus\":")+tostring(static_cast<int>(gamestatus))+
              ",\"frame\":"+tostring(framecounter));
    // #endregion
    if (gamestatus==EXITING)
    {
      // #region agent log
      debug_log("baseline","H15","sopwith.cpp:mainloop","mainloop_break_exit","\"reason\":\"gamestatus_exiting\"");
      // #endregion
      break;
    }
    if (gamestatus==RESTARTING)
      restart();
  }
}

#ifdef DEBUG
std::string stripped(const std::string& s)
{
  for (std::string::size_type i=0;i<s.length();++i)
    if (!isdigit(s[i]))
      return (s.substr(i)=="Remoteplane")?"Playerplane":s.substr(i);
  return std::string("");
}
#endif

void updateobjects()
{
  if (++latencycount>=latency)
    latencycount=0;
  if (gamemode==MULTIPLE && latencycount==0) {
    lastkey=inkeys();
    flushkeybuf();
    putremotekeys(lastkey);
  }
  for (std::list<Object*>::iterator obj=objectlist.begin();obj!=objectlist.end();) {
    #ifdef DEBUG
    Smoke* smoke=dynamic_cast<Smoke*>(*obj);
    Bullet* bullet=dynamic_cast<Bullet*>(*obj);
    if (smoke==0 && bullet==0)
      errorfile<<stripped(typeid(**obj).name())<<':'<<(*obj)->x.integer<<'('<<(*obj)->x.frac<<')'<<','<<(*obj)->y.integer<<'('<<(*obj)->y.frac<<')';
    #endif
    bool exists=(*obj)->update();
    #ifdef DEBUG
    if (smoke==0 && bullet==0)
      errorfile<<"->"<<(*obj)->x.integer<<'('<<(*obj)->x.frac<<')'<<','<<(*obj)->y.integer<<'('<<(*obj)->y.frac<<')'<<std::endl;
    #endif
    /*
    bool lastobject=false;
    if ((++obj)--==objectlist.end())
      lastobject=true;
    */
    objectlist.splice(objectlist.end(),(*obj)->childlist);
    if (!exists) {
      delete *obj;
      obj=objectlist.erase(obj);
    }
    else
      ++obj;
    /*
    if (lastobject)
      break;
    */
  }
  #ifdef DEBUG
  errorfile<<std::endl;
  #endif
  /**/if (gamestatus!=RESTARTING)/**/
  ++framecounter;
}

inline int screenleft()
{
  return std::max(0,std::min(player->x.integer-(units_per_screen_width-PLANE_WIDTH)/2,MAX_X-units_per_screen_width));
}

inline int screendown()
{
  return std::max(0,player->y.integer-(units_per_screen_height+PLANE_HEIGHT)/2);
}

bool onscreen(Object* obj)
{
  int x=(obj->x.integer-screenleft())*zoomx();
  int y=(obj->y.integer-screendown())*zoomy();
  return (x+obj->width*zoomx()  > 0              ) &&
         (x                     < screen_width   ) &&
         (y                     > 0              ) &&
         (y-obj->height*zoomy() < screen_height-1) && obj->sprite!=0;
}

void updatescreen()
{
  static int updatescreenCount=0;
  ++updatescreenCount;
  if (updatescreenCount<=20 || (updatescreenCount%120)==0) {
    // #region agent log
    debug_log("baseline","H7","sopwith.cpp:updatescreen","updatescreen_entry",
              std::string("\"count\":")+tostring(updatescreenCount)+
              ",\"frame\":"+tostring(framecounter));
    // #endregion
  }
  initscreen();
  drawmapground();
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    if (onscreen(*obj)) {
      putimage(((*obj)->x.integer-screenleft())*zoomx(),
               ((*obj)->y.integer-screendown())*zoomy(),(*obj)->sprite,(*obj)->width,(*obj)->height);
      (*obj)->sound();
    }
    if ((*obj)->onmap())
      drawmapobject(screen_center_x()+(std::max(0,std::min(MAX_X-1,(*obj)->x.integer))+(*obj)->width/2)/mapscalex(),
                    (std::max(0,std::min(MAX_Y-1,(*obj)->y.integer))-(*obj)->height/2)/mapscaley(),
                    (*obj)->colour);
  }
  drawground(forceredraw,screenleft(),screendown());
  forceredraw=false;
  displayscore(headplanes[0]);
  if (gamemode==MULTIPLE)
    displayscore(headplanes[1]);
  displayplanesgauge(player);
  displayfuelgauge(player);
  displaybombgauge(player);
  displayammogauge(player);
  if (gameover)
    drawstr(screen_width/2-28,screen_height/2-4,colour_magenta,TRANSPARENT_BACKGROUND,"THE END");
  displayscreen();
  if ((framecounter%60)==0) {
    // #region agent log
    debug_log("baseline","H5","sopwith.cpp:updatescreen","display_called",
              std::string("\"frame\":")+tostring(framecounter)+
              ",\"w\":"+tostring(screen_width)+
              ",\"h\":"+tostring(screen_height));
    // #endregion
  }
}

void checkcollisions()
{
  std::list<Object*> sortedlist(objectlist);
  sortedlist.sort(moretotheleft); /* Stable sort */
  std::list<std::pair<Object*,Object*> > collisionlist;
  collxadj=2;
  collyadj=1;
  if ((framecounter&1)==1) {
    collxadj=-collxadj;
    collyadj=-collyadj;
  }
  const std::list<Object*>::const_iterator end=sortedlist.end();
  for (std::list<Object*>::const_iterator obj=sortedlist.begin();obj!=end;++obj) {
    if ((*obj)->coll==0)
      continue;
    int right=(*obj)->x.integer+(*obj)->width-1;
    int bottom=(*obj)->y.integer;
    int top=bottom-((*obj)->height-1);
    std::list<Object*>::const_iterator test=obj;
    for (++test;test!=end && (*test)->x.integer<=right;++test) {
      if ((*test)->coll==0)
        continue;
      if ((*test)->y.integer>=top && (*test)->y.integer-(*test)->height+1<=bottom && 
          collided(*obj,*test)) {
        Object* highest;
        Object* lowest;
        if ((*obj)->y.integer<(*test)->y.integer) {
          highest=*test;
          lowest=*obj;
        }
        else {
          highest=*obj;
          lowest=*test;
        }
        #ifdef DEBUG
        if ((!instanceof<Frag>(*highest) || !instanceof<Frag>(*lowest) && 
             !instanceof<Smoke>(*highest)) && !instanceof<Smoke>(*lowest))
          errorfile<<'!';
        #endif
        collisionlist.push_back(std::pair<Object*,Object*>(highest,lowest));
      }
    }
    Plane* planeptr;
    if ((((planeptr=dynamic_cast<Plane*>(*obj))!=0 &&
         planeptr->y.integer<ground[planeptr->x.integer+8]+24) ||
        (instanceof<Bomb>(**obj) &&
         (*obj)->y.integer<ground[(*obj)->x.integer+4]+12)) && crashed(*obj)) {
      collisionlist.push_back(std::pair<Object*,Object*>(*obj,0));
    }
  }
  const std::list<std::pair<Object*,Object*> >::const_iterator end2=collisionlist.end();
  for (std::list<std::pair<Object*,Object*> >::const_iterator obj=collisionlist.begin();obj!=end2;++obj) {
    #ifdef DEBUG
      if ((!instanceof<Frag>(*(obj->first)) || !instanceof<Frag>(*(obj->second))) &&
           !instanceof<Smoke>(*(obj->first)) && !instanceof<Smoke>(*(obj->second)))
      errorfile<<stripped(typeid(*(obj->first)).name())<<'('<<(obj->first)->x.integer<<','<<(obj->first)->y.integer<<"):"<<(obj->second!=0?stripped(typeid(*(obj->second)).name()):"Ground")<<std::endl;
    #endif
    (obj->first)->docollision(obj->second);
    objectlist.splice(objectlist.end(),(obj->first)->childlist);
    if (obj->second!=0) {
      #ifdef DEBUG
      if ((!instanceof<Frag>(*(obj->first)) || !instanceof<Frag>(*(obj->second))) &&
           !instanceof<Smoke>(*(obj->first)) && !instanceof<Smoke>(*(obj->second)))
        errorfile<<stripped(typeid(*(obj->second)).name())<<'('<<(obj->second)->x.integer<<','<<(obj->second)->y.integer<<"):"<<(obj->first!=0?stripped(typeid(*(obj->first)).name()):"Ground")<<std::endl;
      #endif
      (obj->second)->docollision(obj->first);
      objectlist.splice(objectlist.end(),(obj->second)->childlist);
    }
  }
  #ifdef DEBUG
  errorfile<<std::endl;
  #endif
}

inline bool moretotheleft(Object* obj1,Object* obj2)
{
  return obj1->x.integer<obj2->x.integer/**/||(obj1->x.integer==obj2->x.integer&&instanceof<Target>(*obj1)&&!instanceof<Target>(*obj2))/**/;
}

bool collided(Object* obj1,Object* obj2)
{
  unsigned int l1,l2,t1,t2,width,height;
  unsigned int widthbytes1,widthbytes2;
  unsigned int y,temp,firstbyte1,firstbyte2,firstbyteinrow1,firstbyteinrow2;
  unsigned int currentbyte1,currentbyte2,lastbyte;
  unsigned char head,bitpos1,bitpos2;
  if (obj1->x.integer > obj2->x.integer) {
    l1 = 0;
    l2 = obj1->x.integer-obj2->x.integer;
  }
  else {
    l1 = obj2->x.integer-obj1->x.integer;
    l2 = 0;
  }
  if (obj1->y.integer < obj2->y.integer) {
    t1 = 0;
    t2 = obj2->y.integer-obj1->y.integer;
  }
  else {
    t1 = obj1->y.integer-obj2->y.integer;
    t2 = 0;
  }
  if (obj1->x.integer+obj1->width < obj2->x.integer+obj2->width) {
    width = obj1->width-l1;
  }
  else {
    width = obj2->width-l2;
  }
  if (obj1->y.integer-obj1->height > obj2->y.integer-obj2->height) {
    height = obj1->height-t1;
  }
  else {
    height = obj2->height-t2;
  }
  widthbytes1 = (obj1->width+7)>>3,
  widthbytes2 = (obj2->width+7)>>3;
  bitpos1 = l1&7;
  bitpos2 = l2&7;
  currentbyte1 = widthbytes1*t1+(l1>>3);
  currentbyte2 = widthbytes2*t2+(l2>>3);
  if ((temp=bitpos1+width) <= 8) {
    head = (1<<temp)-1;
    for (y=0;y<height;++y) {
      if ((((obj1->coll[currentbyte1] & head) >> bitpos1) &
           ((*reinterpret_cast<unsigned short *>(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return true;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
  }
  else {
    firstbyte1 = currentbyte1;
    firstbyte2 = currentbyte2;
    for (y=0;y<height;++y) {
      if (((obj1->coll[currentbyte1] >> bitpos1) &
           ((*reinterpret_cast<unsigned short *>(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return true;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
    ++firstbyte1;
    if (bitpos1 > bitpos2) {
      bitpos2 = 8 - bitpos1 + bitpos2;
    }
    else {
      bitpos2 = bitpos2 - bitpos1;
      ++firstbyte2;
    }
    firstbyteinrow1 = firstbyte1;
    firstbyteinrow2 = firstbyte2;
    lastbyte = firstbyte1+(temp=(width+bitpos1)>>3)-2;
    for (y=0;y<height;++y) {
      for (currentbyte1=firstbyteinrow1,currentbyte2=firstbyteinrow2;
           currentbyte1<=lastbyte;++currentbyte1,++currentbyte2)
        if ((obj1->coll[currentbyte1] &
             ((*reinterpret_cast<unsigned short *>(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
          return true;
      firstbyteinrow1+=widthbytes1;
      firstbyteinrow2+=widthbytes2;
      lastbyte+=widthbytes1;
    }
    if (((l1+width)&7) == 0) {
      return false;
    }
    head = (1<<(((l1+width-1)&7)+1)) - 1;
    currentbyte1 = firstbyte1+temp-1;
    currentbyte2 = firstbyte2+temp-1;
    for (y=0;y<height;++y) {
      if (((obj1->coll[currentbyte1] & head) &
           ((*reinterpret_cast<unsigned short *>(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return true;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
  }
  return false;
}

bool crashed(Object* obj)
{
  int byte,widthbytes;
  char bit;
  int y;
  widthbytes=(obj->width+7)>>3;
  for (byte=0;byte<widthbytes;++byte)
    for (bit=0;bit<8 && (byte<<3)+bit < obj->width;++bit) {
      y=obj->y.integer-static_cast<int>(ground[(byte<<3)+bit+obj->x.integer]);
      if (y<0)
        return true;
      if (y>=obj->height)
        continue;
      if ((obj->coll[y*widthbytes+byte] & (1<<bit)) != 0)
        return true;
    }
  return false;
}

void restart()
{
  if (player->endstatus==Plane::WINNER) {
    int sc=0;
    while (player->lives-->0) {
      sc+=25;
      player->score+=sc;
      displayplanesgauge(player);
      displayscore(player);
      updatescreen();
      #ifndef DEBUG
      #ifndef DEBUG2
      unsigned long timerstart=timer();
      do {
        processtimerticks();
        flushkeybuf();
      } while (timer()-timerstart<5);
      #endif
      #endif
    }
    if (level<=5)
      ++level;
    savescore=player->score;
  }
  else
    #ifdef DEBUG2
    {throw sw_excep(player->score);
    #endif
    level=savescore=0;
    #ifdef DEBUG2
    }
    #endif
  gameover=false;
  initlevel();
}

int history(int keys)
{
  int extrakeys=keys&(KEY_SOUND|KEY_PAUSEGAME);
  keys&=~extrakeys;
  if (!playbackfilename.empty()) {
    if (keys!=0)
      playbackfilename=""; /* playbackfilename.clear(); */
    else {
      try {
        int tempkeys=getshort(inputfile);
        if (tempkeys==framecounter)
          keys=getshort(inputfile);
        else {
          putbackshort(inputfile,tempkeys);
          keys=historykeys;
        }
      }
      catch(...) {
        replay_visual_log_note("playback_stream_end_or_error");
        playbackfilename=""; /* playbackfilename.clear(); */
      }
    }
  }
  if (!recordfilename.empty() && keys!=historykeys) {
    putshort(outputfile,framecounter);
    putshort(outputfile,keys);
  }
  historykeys=keys;
  return keys|extrakeys;
}

void loser(Plane* plane)
{
  plane->endstatus=Plane::LOSER;
  if (plane==player)
    gameover=true;
  plane->endcounter=20;
}

void winner(Plane* plane)
{
  plane->endstatus=Plane::WINNER;
  plane->endcounter=72;
  plane->xv=plane->yv=0;
  plane->state=Plane::FLYING;
  plane->life=MAX_FUEL;
  plane->speed=MIN_SPEED;
}

void endgame(int targetcolour)
{
  if (gamemode!=MULTIPLE && targetcolour==player->colour)
    return;
  int wincolour=(colour_cyan+colour_magenta)-targetcolour;
  const std::list<Object*>::const_iterator end=objectlist.end();
  for (std::list<Object*>::const_iterator obj=objectlist.begin();obj!=end;++obj) {
    Plane* plane=dynamic_cast<Plane*>(*obj);
    if (plane!=0) {
      if (plane->endstatus==Plane::NOTFINISHED) {
        if (plane->colour==wincolour &&
            (plane->lives>1 ||
             (plane->lives>0 &&
              (plane->active()))))
          winner(plane);
        else
          loser(plane);
      }
    }
  }
}

#endif /* SOPWITH_SOPWITH_CPP */
