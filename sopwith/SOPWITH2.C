/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2001 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <alloc.h>
#include <string.h>
#include <conio.h>
#include <stdarg.h>
#include "def.h"
#include "sopasm.h"

typedef struct tonetable { /* Continuous tone table entry */
  unsigned int tone;
  int change;
  struct tonetable* next;
  struct tonetable* prev;
} tonetable;

typedef struct object {
  int state;
  int x,y,xv,yv,angle;
  int subtype;
  bool inverted;
  int speed,accel,flaps;
  bool firing;
  short score; int ammo;
  int hitcounter;
  int life;
  struct object* owner;
  int height,width;
  bool bombing;
  int bombs,colour;
  unsigned short xfrac,yfrac,xvfrac,yvfrac;
  struct object* next;
  struct object* prev;
  int index;
  int oldx,oldy;
  bool drawflag,deleteflag;
  unsigned char* oldsprite;
  void (*display_func)(struct object* obj);
  bool (*update_func)(struct object* obj);
  struct object* nextx;
  struct object* prevx;
  int lives;
  unsigned char* sprite;
  unsigned char* coll;
  int bomb_delay;
  bool goinghome;
  int type;
  bool athome;
  tonetable* tonetab;
  int missiles;
  struct object* missile_target;
  int missile_delay;
  struct object* target;
  int starbursts;
  int starburst_firing;
  int starburst_delay;
} object;

typedef struct {
  int colour,x,y;
} mapobject;

typedef struct { /* Communications buffer */
  unsigned maxplayers;
  unsigned players;
  unsigned lastplayer;
  unsigned keys[MAX_PLAYERS];
  unsigned states[MAX_PLAYERS];
  unsigned randv;
} netbuffer;

netbuffer netbuf;

int writecharcol=3;
int latency=-1; /* Number of displays per keystroke */
int sine[ANGLES]={ /* Sine table of pi/8 increments times 0x100 */
  0x000, 0x062, 0x0b5, 0x0ed, 0x100, 0x0ed, 0x0b5, 0x062,
  0x000,-0x062,-0x0b5,-0x0ed,-0x100,-0x0ed,-0x0b5,-0x062};
int keysprev,joykeysprev;
volatile int keyspressed,keysnext;
int joykeyspressed,joykeysnext;
volatile bool keypressedflag;

ground_t screenground[SCR_WIDTH];
int soundtype=SOUND_NONE; /* Current sound type and */
int soundparam=0x7fff;    /* priority parameter */
object* soundobj=NULL; /* Object making sound */
unsigned int lastfreq=0; /* Last frequency used */
object* lastobj=NULL; /* Previous object making sound */
void (*soundadjfunc)(void)=NULL; /* Tone adjustment on clock tick */
char* sopwith2tune[]={
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
  "e4/g8/g2/r16/>a8/<g8/e8/d2./",
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
  "e4/>a8/a2/r16/<g8/f+8/e8/d2./",
  "d8/g2/r16/g8/g+2/r16/g+8/>a2/r16/a8/c2/r16/",
  "b8/a8/<g8/>b4/<g8/>b4/<g8/>a4./<g1/",
  ""};

#include "ground.c"
#include "sprites.c"
#include "coll.c"

short huge* historybuf;
long historyspace;
FILE* outputfile;
short huge* historyp;
int historykeys;
long historyfilelen;
bool recording;

int gamemode;
bool multiplayer,truemultiplayer;
object* targets[MAX_AVOIDABLES];
int numtargets[2]; /* Number of active targets by colour */

volatile unsigned long timertick=0;
unsigned long processedtimerticks=0;
int framecounter=0;
int speedtick=0;
int level=0;
int minspeed,maxspeed; /* Speed range based on level */
int targetrange2;      /* Target range based on level */

unsigned char auxbuf[0x2000];

bool notitlesflag;
bool soundflag=TRUE,joystick,ibmkeyboard,inplay=FALSE;

int screenleft,pixtoscroll;
unsigned int interlacediff;
unsigned char far* scrp;
bool groundtobedrawn;

object* objects;
object originalplanes[MAX_PLAYERS];
object* lastobject;
object* firstobject;
object* nextfreeobject;
object* firstdeleted;
object* lastdeleted;
object objleft,objright;
object* compnear[MAX_PLAYERS]; /* Planes near computer planes */
int endstatuses[MAX_PLAYERS],endcounter=1,playerindex=0,planeindex;
bool planeisplayer,planeiscomputer;
mapobject mapobjects[MAX_OBJECTS];
bool flyingintosunset,forceredraw;
char* playbackfilename;
char* recordfilename;
#if VERSION==2
int randv;
#else
long int randv;
#endif
volatile bool breakf;
int latencycount; /* Displays to delay keyboard */
int endstatus,maxlives;
bool restarting=FALSE;

bool exiting=FALSE;
tonetable tonetab[SOUNDSIZE]; /* Continuous tone table */
tonetable* firsttone;
tonetable* freetone;
int soundticks;
int numfrags;
bool titleflag;
int explline,explplace,explticks,exploctavemul;
unsigned int expltone;
int titleline,titleplace,titleticks,titleoctavemul;
unsigned int titletone;
int tuneline,tuneplace,tunefreq,tunedur,octavemultiplier;
char** tune;
bool ghost;
int savescore;
object* collobj1[MAX_OBJECTS<<1];
object* collobj2[MAX_OBJECTS<<1];
int killptr;
int collxv[MAX_PLAYERS],collyv[MAX_PLAYERS];
object* collobjs[MAX_PLAYERS];
int collptr,collxadj,collyadj;
ground_t ground[MAX_X];
int courseadjust;
bool splatox,oxsplatted;
int shothole,splatbird;

int lastkey /* =0 */ ;

volatile bool paused;
bool missok=FALSE;

int main(int argc,char* argv[]);
void keybint(void);
int inkeys(void);
void updatejoy(void);
int readjoychannel(int channel);
void flushkeybuf(void);
void setcolour(int c);
void updatescreen(void);
void drawground(ground_t* screenground);
void drawobject(int x,int y,object* obj);
void putpixel(int x,int y,int c);
int pixel(int x,int y,int c);
void putimage(int x,int y,unsigned char* p,object* obj);
void updateobjects(void);
bool updateplayerplane(object* plane);
void processkeys(object* p,int keys);
bool updatecomputerplane(object* plane);
void putremotekeys(void);
bool updateplane(object* plane);
void alertcomputer(object* obj);
void refuel(object* plane);
bool topup(int* val,int max);
bool updatebullet(object* bullet);
bool updatebomb(object* bomb);
int direction(object* obj);
bool updatetarget(object* target);
bool updatefrag(object* frag);
bool updatesmoke(object* smoke);
bool updateflock(object* flock);
bool updatebird(object* bird);
bool updateox(object* ox);
void crashplane(object* plane);
void hitplane(object* plane);
void stallplane(object* plane);
void insertx(object* ins,object* list);
void deletex(object* del);
void inittonetable(void);
void setsound(int type,int param,object* obj);
void updatesound(void);
void soundadjust(void);
void soundadjslide(void);
void soundadjshot(void);
void explnote(void);
void updatetune3(void);
void titlenote(void);
void initsound(object* obj,int sn);
tonetable* alloctone(void);
void dealloctone(tonetable* s);
void stopsound(object* o);
void tone(unsigned int t2);
void soundoff(void);
int soundrand(int f);
void playnote(void);
void init(int argc,char* argv[]);
void initrand(void);
void restart(void);
void initdifficulty(void);
void clearwin(void);
void getcontrol(void);
int getgamemode(void);
int getgamenumber(void);
bool testbreak(void);
void copyground(void);
void initdisplay(bool drawnground);
void initscores(void);
void displayplanesgauge(object* p);
void displayfuelgauge(object* p);
void displaybombgauge(object* p);
void displayammogauge(object* p);
void displaymissilegauge(object* player);
void displaystarburstgauge(object* player);
void displaygauge(int x,int h,int hmax,int col);
void drawmapground(void);
void drawmaptargets(void);
void useauxbuf(void);
void usescreenbuf(void);
void clearauxbuf(void);
void initobjects(void);
void createcomputerplane(object* pl);
void createplayerplane(object* player);
object* createplane(object* pl);
void createbullet(object* obj,object* at);
int range2(int x0,int y0,int x1,int y1);
void createbomb(object* plane);
void createmissile(object* plane);
void createstarburst(object* plane);
void createtargets(void);
void createexplosion(object* obj);
void createsmoke(object* plane);
void createflocks(void);
void createbird(object* flock,int birdno);
void createoxen(void);
void checkcollisions(void);
bool checkcollision(object* obj1,object* obj2);
bool checkcrash(object* obj);
void docollision(object* obj1,object* obj2);
bool scorepenalty(int type,object* obj,int score);
void scoretarget(object* destroyed,int score);
void scoreplane(object* destroyed);
void displayscore(object* p);
void crater(object* obj);
void titles(void);
void timerint(void);
void processtimerticks(void);
void displaybomb(object* bomb);
void displayplayerplane(object* plane);
void displaymissile(object* missile);
void displaystarburst(object* starburst);
void displayfrag(object* frag);
void displaycomputerplane(object* plane);
void displayremoteplane(object* plane);
void displaytarget(object* target);
void displayflock(object* flock);
void displaybird(object* bird);
void soundtarget(object* target);
void soundplane(object* plane);
void drawmapobject(object* obj);
void finish(char* msg);
void endgame(int n);
void winner(object* player);
void loser(object* plane);
void eogstats(void);
object* allocobj(void);
void deleteobject(object* obj);
void computerpilot(object* plane);
void attack(object* plane,object* attackee);
void cruise(object* plane);
void gohome(object* plane);
int gunontarget(object* target);
void aim(object* plane,int destx,int desty,object* destobj,bool recursing);
void inittargetcheck(int x);
bool doomed(object* plane,int x,int y,int alt);
int range(int x0,int y0,int x1,int y1);
int getmaxplayers(void);
bool updateremoteplane(object* plane);
int inithistory(int randv);
void freehistorybuf(void);
void freeandexit(char* errmsg);
int history(int key);
void flushhistory(void);
int getremotekeys(object* plane);
void setvel(object* obj,int v,int dir);
void printstr(char* str);
void moveobject(object* obj,int* x,int* y);
void getflags(int* argc,char** argv[],char* format,...);
void writenum(int n,int width);
void initasynch(void);
void createasynchremoteplane(void);
int inkey(void);
void dispwindshot(void);
void dispsplatbird(void);

void readnet(void) {}
void writenet(void) {}
void opennet(void) {}
void closenet(void) {}

#undef min
#undef max

int min(int a,int b)
{
  return (a<b) ? a : b;
}

int max(int a,int b)
{
  return (a>b) ? a : b;
}

int main(int argc,char* argv[])
{
  int nmodes,ncontrols,i;
  bool modes=FALSE,modec=FALSE,modem=FALSE,modea=FALSE,keybflag=FALSE;
  objects=(object*)malloc(MAX_OBJECTS*sizeof(object));
  if (objects==NULL) {
    printf("Cannot allocate memory for object list.\n");
    exit(1);
  }
  delay(0);
  soundflag=FALSE;
  getflags(&argc,&argv,"y#q&g#s&c&m&a&j&k&i&h*v*x&:sopwith2*",
         &latency,          /* -y[num] */
         &soundflag,        /* -q (quiet) */
         &level,            /* -g[num] */
         &modes,            /* -s */
         &modec,            /* -c */
         &modem,            /* -m */
         &modea,            /* -a */
         &joystick,         /* -j */
         &keybflag,         /* -k */
         &ibmkeyboard,      /* -i */
         &recordfilename,   /* -h[string] (record) */
         &playbackfilename, /* -v[string] (playback) */
         &missok            /* -x */
        );
  soundflag=!soundflag;
  nmodes=(modes ? 1 : 0)+(modec ? 1 : 0)+(modem ? 1 : 0)+(modea ? 1 : 0);
  if (nmodes>1) {
    printstr("\r\nOnly one mode: -s -c -m -a  may be specified\r\n");
    exit(1);
  }
  gamemode=(modes ? SINGLE : (modec ? COMPUTER : (modem ? MULTIPLE : ASYNCH)));
  ncontrols=(keybflag ? 1 : 0)+(joystick ? 1 : 0);
  if (ncontrols>1) {
    printstr("\r\nOnly one of -j and -k may be specified\r\n");
    exit(1);
  }
  if (nmodes>0 && ncontrols>0)
    notitlesflag=TRUE;
  initrand();
  randv=inithistory(randv);
  inittonetable();
  inittimer(timerint);

  setgmode(BIOS_GRAPHICS);
  if (!notitlesflag) {
    setsound(SOUND_TITLE,0,NULL);
    updatesound();
    setcolour(3); poscurs(13,8); printstr("S O P W I T H");
    setcolour(1); poscurs(12,11); printstr("BMB "); setcolour(3);
    printstr("Compuscience");

    if (nmodes==0)
      gamemode=getgamemode();
    if (ncontrols==0)
      getcontrol();
  }
  multiplayer=(gamemode==MULTIPLE || gamemode==ASYNCH);
  if (multiplayer) {
    maxlives=MAXLIVES*2;
    copyground();
    initobjects();
    if (gamemode==ASYNCH)
      createasynchremoteplane();
    truemultiplayer=netbuf.maxplayers>1;
    createtargets();
    initdisplay(FALSE);
    if (latency==-1)
      latency=1;
  }
  else {
    if (latency==-1)
      latency=1;
    maxlives=MAXLIVES;
    clearwin();
    copyground();
    initobjects();
    createplayerplane(NULL);
    for (i=0;i<3;i++)
      createcomputerplane(NULL);
    truemultiplayer=FALSE;
    createtargets();
    initdisplay(FALSE);
  }
  createflocks();
  createoxen();
  initdifficulty();
  initkeyboard(keybint);
  inplay=TRUE;

  while (TRUE) {
    speedtick=0;
    while (!restarting) {
      do {
        processtimerticks();
      } while (speedtick<SPEED);
      speedtick-=SPEED;
      updateobjects();
      updatejoy();
      updatescreen();
      updatejoy();
      checkcollisions();
      updatejoy();
      updatesound();
    }
    restart();
  }
}

void keybint(void)
{
  int scancode,k;
  keypressedflag=TRUE;
  if (!ibmkeyboard)
    return;
  paused=FALSE;
  scancode=inportb(PORT_KEYB);
  switch (scancode&0x7f) {
    case SC_X:     k=KEY_ACCEL;     break;
    case SC_Z:     k=KEY_BRAKE;     break;
    case SC_COMMA: k=KEY_CLIMB;     break;
    case SC_SLASH: k=KEY_DESCEND;   break;
    case SC_DOT:   k=KEY_FLIP;      break;
    case SC_SPACE: k=KEY_FIRE;      break;
    case SC_B:     k=KEY_BOMB;      break;
    case SC_H:     k=KEY_HOME;      break;
    case SC_S:     k=KEY_SOUND;     break;
    case SC_BREAK: k=KEY_BREAK;     breakf=TRUE; break;
    case SC_V:     k=KEY_MISSILE;   break;
    case SC_C:     k=KEY_STARBURST; break;
    case SC_P:
#if 0
      if (scancode&0x80) {
        paused=TRUE;
        if (soundflag) {
          sound(SOUND_OFF,0,NULL);
          updatesound();
          soundflag=FALSE;
          while(paused);
          soundflag=TRUE;
        }
        else
          while(paused);
      }
#endif
      k=KEY_PAUSE;
      break;
    default: k=0;
  }
  if (k!=0)
    if ((scancode&0x80)!=0) {
      if ((k&keysprev)!=0)
        keysnext&=~k;
      keyspressed&=~k;
    }
    else {
      keyspressed|=k;
      keysnext|=k;
    }
}

int inkeys(void)
{
  int k;
  if (!inplay)
    return history(inkey());
  if (ibmkeyboard) {
    k=keysprev=keysnext;
    keysnext=keyspressed;
    while (kbhit()) getkey();
  }
  else {
    switch(inkey()) {
      case 'X': case 'x': k=KEY_ACCEL;     break;
      case 'Z': case 'z': k=KEY_BRAKE;     break;
      case '<': case ',': k=KEY_CLIMB;     break;
      case '?': case '/': k=KEY_DESCEND;   break;
      case '>': case '.': k=KEY_FLIP;      break;
      case ' ':           k=KEY_FIRE;      break;
      case 'B': case 'b': k=KEY_BOMB;      break;
      case 'H': case 'h': k=KEY_HOME;      break;
      case 'S': case 's': k=KEY_SOUND;     break;
      case 'V': case 'v': k=KEY_MISSILE;   break;
      case 'C': case 'c': k=KEY_STARBURST; break;
      case 'P': case 'p': k=KEY_PAUSE;     break;
      default: k=0;
    }
    if (testbreak())
      k|=KEY_BREAK;
  }
  if (joystick) {
    joykeysprev=joykeysnext;
    joykeysnext=joykeyspressed;
    k|=joykeysprev;
  }
  return history(k);
}

int inkey(void)
{
  if (kbhit())
    return getkey();
  return 0;
}

void updatejoy(void)
{
  static int joykeys[9]={KEY_FLIP, KEY_DESCEND,KEY_FLIP,
                         KEY_BRAKE,0,          KEY_ACCEL,
                         KEY_FLIP, KEY_CLIMB,  KEY_FLIP};

  int x,y,j,r;
  if (joystick) {
    x=readjoychannel(JOY_X);
    y=readjoychannel(JOY_Y);
    j=joykeys[(x<=JOYLOW ? 0 : (x>=JOYHIGH ? 2 : 1))+
              (y<=JOYLOW ? 0 : (y>=JOYHIGH ? 6 : 3))];
    r=inportb(PORT_JOY);
    if ((r&JOY_BUT1)==0)
      j|=KEY_FIRE;
    if ((r&JOY_BUT2)==0)
      j|=KEY_BOMB;
    joykeyspressed=j;
    joykeysnext|=j;
    joykeysnext&=j|~joykeysprev;
  }
}

int readjoychannel(int channel)
{
  int t;
  if ((inportb(PORT_JOY)&channel)!=0)
    return JOYMAX;
  keypressedflag=FALSE;
  outportb(PORT_TIMERC,0);
  t=getwordfromport(PORT_TIMER0);
  outportb(PORT_JOY,0);
  while ((inportb(PORT_JOY)&channel)!=0);
  if (keypressedflag)
    return (JOYHIGH+JOYLOW)/2;
  return t-getwordfromport(PORT_TIMER0);
}

void flushkeybuf(void)
{
/*  if (!inplay || !ibmkeyboard) */
    while (kbhit())
      getkey();
}

void setcolour(int c)
{
  writecharcol=c;
}

void updatescreen(void)
{
  object* obj;
  int i;
  usescreenbuf();
  for (obj=firstobject;obj!=NULL;obj=obj->next)
    if (obj->deleteflag && obj->drawflag && obj->height!=1 &&
        obj->oldsprite==obj->sprite && obj->y==obj->oldy &&
        obj->oldx+screenleft==obj->x) {
      if (obj->display_func!=NULL)
        obj->display_func(obj);
    }
    else {
      if (obj->deleteflag)
        putimage(obj->oldx,obj->oldy,obj->oldsprite,obj);
      if (obj->drawflag) {
        if (obj->x>=screenleft && obj->x<=screenleft+SCR_WIDTH-1 &&
            (!oxsplatted || (obj->x-screenleft)/(SCR_WIDTH/3+1)==1)) {
          obj->oldx=obj->x-screenleft;
          obj->oldy=obj->y;
          putimage(obj->oldx,obj->oldy,obj->sprite,obj);
          if (obj->display_func!=NULL)
            obj->display_func(obj);
        }
        else
          obj->drawflag=FALSE;
      }
    }
  for (obj=firstdeleted;obj!=NULL;obj=obj->next)
    if (obj->deleteflag)
      putimage(obj->oldx,obj->oldy,obj->oldsprite,obj);
  if (!oxsplatted) {
    if (groundtobedrawn || pixtoscroll!=0 || forceredraw) {
      if (!groundtobedrawn)       /* If the ground doesn't need drawing */
        drawground(screenground); /* delete it first */
      memcpy(screenground,ground+screenleft,SCR_WIDTH*sizeof(ground_t));
      drawground(screenground);
    }
    if (splatox) {
      farmemset(MK_FP(SCR_SEG,0),MAGENTA4,
                ((SCR_HEIGHT-SCR_SCROLLBOT-2)>>1)*SCR_BYTES);
      farmemset(MK_FP(SCR_SEG,SCR_INTERLACE),MAGENTA4,
                ((SCR_HEIGHT-SCR_SCROLLBOT-3)>>1)*SCR_BYTES);
      splatox=FALSE;
      oxsplatted=TRUE;
      for (i=0,obj=objects;i<MAX_OBJECTS;i++,obj++) {
        obj->drawflag=FALSE;
        obj->deleteflag=FALSE;
      }
    }
  }
  groundtobedrawn=FALSE;
  forceredraw=FALSE;
}

void drawground(ground_t* screenground)
{
  int y=*screenground,y2;
  int x;
  for (x=0;x<SCR_WIDTH;x++) {
    y2=*(screenground++);
    if (y2==y)
      putpixel(x,y,XOR_WHITE);
    else
      if (y2>y)
        do
          putpixel(x,y++,XOR_WHITE);
        while (y!=y2);
      else
        do
          putpixel(x,y--,XOR_WHITE);
        while (y!=y2);
  }
}

void updateobjects(void)
{
  object* next;
  object* current;
  if (firstdeleted!=NULL) {
    lastdeleted->next=nextfreeobject;
    nextfreeobject=firstdeleted;
    lastdeleted=firstdeleted=NULL;
  }
  latencycount++;
  if (latencycount>=latency)
    latencycount=0;
  if (multiplayer && latencycount==0)
    putremotekeys();
  for (current=firstobject;current!=NULL;current=next) {
    next=current->next;
    current->deleteflag=current->drawflag;
    current->oldsprite=current->sprite;
    current->drawflag=current->update_func(current);
    /*
    if (multiplayer && current->index==netbuf.maxplayers && latencycount==0)
      putremotekeys();
    */
  }
  framecounter++;
}

bool updateplayerplane(object* plane)
{
  bool result;
  int oldx,keys;
  planeiscomputer=FALSE;
  planeisplayer=TRUE;
  planeindex=plane->index;
  endstatus=endstatuses[playerindex];
  if (endstatus!=NOTFINISHED) { /* && endcounter>0) { */
    endcounter--;
/* #if VERSION!=7 */
    if (endcounter<=0) {
      if (!multiplayer && !exiting)
        restarting=TRUE;
      else
        finish(NULL);
    }
/* #endif */
  }
  if (latencycount==0) {
    if (multiplayer)
      keys=getremotekeys(plane);
    else {
      keys=inkeys();
      flushkeybuf();
    }
    processkeys(plane,keys);
  }
  else {
    plane->flaps=0;
    plane->bombing=FALSE;
    plane->starburst_firing=FALSE;
    plane->missile_target=NULL;
  }
  if ((plane->state==CRASHED || plane->state==GHOSTCRASHED) &&
      plane->hitcounter<=0) {
    plane->lives--;
    if (endstatus!=WINNER && (plane->life<=QUIT || (!multiplayer &&
                                                    plane->lives<=0))) {
      if (endstatus==NOTFINISHED)
        loser(plane);
    }
    else {
      createplayerplane(plane);
      initdisplay(TRUE);
      if (endstatus==WINNER)
        winner(plane);
    }
  }
  oldx=plane->x;
  result=updateplane(plane);
  if (oldx<=SCROLL_LIMIT || oldx>=MAX_X-SCROLL_LIMIT)
    pixtoscroll=0;
  else {
    pixtoscroll=plane->x-oldx;
    screenleft+=pixtoscroll;
  }
  if (!plane->athome) {
    usescreenbuf();
    if (plane->firing)
      displayammogauge(plane);
    if (plane->bombing)
      displaybombgauge(plane);
#if VERSION==7
    if (plane->missile_target!=NULL)
      displaymissilegauge(plane);
    if (plane->starburst_firing)
      displaystarburstgauge(plane);
#endif
  }
  return result;
}

void processkeys(object* plane,int keys)
{
  int state=plane->state;
  plane->flaps=0;
  plane->bombing=FALSE;
  plane->starburst_firing=FALSE;
  plane->missile_target=NULL;
  plane->firing=FALSE;
  if (state!=FLYING && state!=STALLED && state!=FALLING && state!=WOUNDED &&
      state!=WOUNDSTALL && state!=GHOST && state!=GHOSTSTALLED)
    return;
  if (state!=FALLING) {
    if (endstatus!=NOTFINISHED) {
      if (endstatus==LOSER && planeisplayer)
        gohome(plane);
      return;
    }
    if ((keys&KEY_BREAK)!=0) {
      plane->life=QUIT;
      plane->goinghome=FALSE;
      if (plane->athome) {
        plane->state=state=(state>=FINISHED) ? GHOSTCRASHED : CRASHED;
        plane->hitcounter=0;
      }
      if (planeisplayer)
        exiting=TRUE;
    }
    if ((keys&KEY_HOME)!=0 &&
        (state==FLYING || state==GHOST || state==WOUNDED))
      plane->goinghome=TRUE;
  }
  if ((framecounter&1)==1 || (state!=WOUNDED && state!=WOUNDSTALL)) {
    if ((keys&KEY_CLIMB)!=0) {
      plane->flaps++;
      plane->goinghome=FALSE;
    }
    if ((keys&KEY_DESCEND)!=0) {
      plane->flaps--;
      plane->goinghome=FALSE;
    }
    if ((keys&KEY_FLIP)!=0) {
      plane->inverted=!plane->inverted;
      plane->goinghome=FALSE;
    }
    if ((keys&KEY_BRAKE)!=0) {
      if (plane->accel)
        plane->accel--;
      plane->goinghome=FALSE;
    }
    if ((keys&KEY_ACCEL)!=0) {
      if (plane->accel<MAX_THROTTLE)
        plane->accel++;
      plane->goinghome=FALSE;
    }
  }
  if ((keys&KEY_FIRE)!=0 && state<FINISHED)
    plane->firing=TRUE;
  if ((keys&KEY_MISSILE)!=0 && state<FINISHED)
    plane->missile_target=plane;
  if ((keys&KEY_BOMB)!=0 && state<FINISHED)
    plane->bombing=TRUE;
  if ((keys&KEY_STARBURST)!=0 && state<FINISHED)
    plane->starburst_firing=TRUE;
  if ((keys&KEY_SOUND)!=0 && planeisplayer) {
    if (soundflag) {
      setsound(SOUND_OFF,0,NULL);
      updatesound();
    }
    soundflag=!soundflag;
  }
  if (plane->goinghome)
    gohome(plane);
}

bool updatecomputerplane(object* plane)
{
  planeiscomputer=TRUE;
  planeisplayer=FALSE;
  plane->flaps=0;
  plane->starburst_firing=FALSE;
  plane->bombing=FALSE;
  plane->missile_target=NULL;
  planeindex=plane->index;
  endstatus=endstatuses[planeindex];
  if (latencycount==0)
    plane->firing=FALSE;
  switch (plane->state) {
    case WOUNDED:
    case WOUNDSTALL:
      if ((framecounter&1)==1)
        break;
    case FLYING:
    case STALLED:
      if (endstatus!=NOTFINISHED)
        gohome(plane);
      else
        if (latencycount==0)
          computerpilot(plane);
      break;
    case CRASHED:
      plane->firing=FALSE;
      if (plane->hitcounter<=0 && endstatus==NOTFINISHED)
        createcomputerplane(plane);
      break;
    default:
      plane->firing=FALSE;
  }
  return updateplane(plane);
}

void updated(int n1,int n2)
{
  int n;
  bool done;
  int last;

  for (;;) {
    last=netbuf.lastplayer;
    done=TRUE;
    for (n=n1;n<n2;n++)
      if (netbuf.states[n]!=FINISHED)
        break;
    if (n<n2)
      if (playerindex==last)
        done=FALSE;
      else
        if (last>=n)
          for (;n<n2;n++)
            if (netbuf.states[n]!=FINISHED)
              done=(n==last);
    if (done)
      return;
    readnet();
  }
}

void putremotekeys(void)
{
  /* static bool first=TRUE; */
  if (gamemode==MULTIPLE)
    updated(0,playerindex);
  /*
  if (first)
    first=FALSE;
  else
  */
    lastkey=inkeys();
  flushkeybuf();
  if (gamemode==MULTIPLE) {
    netbuf.keys[playerindex]=lastkey;
    netbuf.states[playerindex]=objects[playerindex].state;
    netbuf.lastplayer=playerindex;
    writenet();
    updated(playerindex+1,netbuf.maxplayers);
  }
  else
    ; /* Output "lastkey" to port */
}

bool updateplane(object* plane)
{
  static char gravity[16]={0,-1,-2,-3,-4,-3,-2,-1,0,1,2,3,4,3,2,1};

  int x,y,newangle,newspeed,state=plane->state,newstate,limitspeed;
  bool update,stalled;
  switch (state) {
    case FINISHED:
    case WAITING:
      return FALSE;
    case CRASHED:
    case GHOSTCRASHED:
      plane->hitcounter--;
      break;
    case FALLING:
      plane->hitcounter-=2;
      if (plane->yv<0 && plane->xv!=0)
        if (((plane->inverted ? 1 : 0)^(plane->xv<0 ? 1 : 0))!=0)
          plane->hitcounter-=plane->flaps;
        else
          plane->hitcounter+=plane->flaps;
      if (plane->hitcounter<=0) {
        if (plane->yv<0)
          if (plane->xv<0)
            plane->xv++;
          else
            if (plane->xv>0)
              plane->xv--;
            else
              plane->inverted=!plane->inverted;
        if (plane->yv>TERMINAL_VELOCITY)
          plane->yv--;
        plane->hitcounter=FALLCOUNT;
      }
      plane->angle=direction(plane)<<1;
      if (plane->yv<=0)
        initsound(plane,SOUND_FALLING);
      break;
    case STALLED:
    case GHOSTSTALLED:
    case WOUNDSTALL:
    case FLYING:
    case WOUNDED:
    case GHOST:
      if (state==STALLED)
        newstate=FLYING;
      if (state==GHOSTSTALLED)
        newstate=GHOST;
      if (state==WOUNDSTALL)
        newstate=WOUNDED;
      if (state==STALLED || state==GHOSTSTALLED || state==WOUNDSTALL) {
        stalled=(plane->angle!=3*ANGLES/4 || plane->speed<minspeed);
        if (!stalled)
          plane->state=state=newstate;
      }
      else {
        stalled=(plane->y>=MAX_Y);
        if (stalled)
          if (gamemode==NOVICE) {
            plane->angle=(3*ANGLES/4);
            stalled=FALSE;
          }
          else {
            stallplane(plane);
            state=plane->state;
          }
      }
      if (flyingintosunset)
#if VERSION!=2
       if (planeisplayer)
#endif
        break;
      if (plane->life<=0 && !plane->athome &&
          (state==FLYING || state==STALLED || state==WOUNDED ||
           state==WOUNDSTALL)) {
        hitplane(plane);
        scoreplane(plane);
        return updateplane(plane);
      }
#if VERSION!=2
      if (plane->firing)
        createbullet(plane,NULL);
      if (plane->bombing)
        createbomb(plane);
      if (plane->missile_target!=NULL)
        createmissile(plane);
      if (plane->starburst_firing)
        createstarburst(plane);
#endif
      newangle=plane->angle;
      newspeed=plane->speed;
      update=FALSE;
      if (plane->flaps!=0) {
        if (plane->inverted)
          newangle-=plane->flaps;
        else
          newangle+=plane->flaps;
        newangle&=(ANGLES-1);
        update=TRUE;
      }
      if ((framecounter&3)==0)
        if (!stalled && newspeed<minspeed && gamemode!=NOVICE) {
          newspeed--;
          update=TRUE;
        }
        else {
          limitspeed=minspeed+plane->accel+gravity[newangle];
          if (newspeed<limitspeed) {
            newspeed++;
            update=TRUE;
          }
          else
            if (newspeed>limitspeed) {
              newspeed--;
              update=TRUE;
            }
        }
      if (update) {
        if (plane->athome)
          if (plane->accel!=0 || plane->flaps!=0)
            newspeed=minspeed;
          else
            newspeed=0;
        else
          if (newspeed<=0 && !stalled) {
            if (gamemode==NOVICE)
              newspeed=1;
            else {
              stallplane(plane);
              return updateplane(plane);
            }
          }
        plane->speed=newspeed;
        plane->angle=newangle;
        if (stalled) {
          plane->xv=plane->xvfrac=plane->yvfrac=0;
          plane->yv=-newspeed;
        }
        else
          setvel(plane,newspeed,newangle);
      }
      if (stalled) {
        plane->hitcounter--;
        if (plane->hitcounter==0) {
          plane->inverted=!plane->inverted;
          plane->angle=(ANGLES/2-plane->angle)&(ANGLES-1);
          plane->hitcounter=STALLCOUNT;
        }
      }
#if VERSION==2
      if (plane->firing)
        createbullet(plane,NULL);
      if (plane->bombing)
        createbomb(plane);
      if (plane->missile_target!=NULL)
        createmissile(plane);
      if (plane->starburst_firing)
        createstarburst(plane);
#endif
      if (!planeiscomputer) {
        if (planeisplayer &&
            plane->speed>plane->life%(MAX_FUEL/GAUGE_HEIGHT)) {
          usescreenbuf();
          displayfuelgauge(plane);
        }
        plane->life-=plane->speed;
      }
      if (plane->speed!=0)
        plane->athome=FALSE;
      break;
  }
  if (endstatus==WINNER && planeisplayer && flyingintosunset) {
    plane->sprite=finalesprites[plane->colour==MOV_CYAN ? 0 : 1]
                               [endcounter/ENDTIME];
    plane->coll=finalespritescoll[endcounter/ENDTIME];
  }
  else
    if (plane->state==FINISHED) {
      plane->sprite=NULL;
      plane->coll=NULL;
    }
    else
      if (plane->state==FALLING && !plane->xv && plane->yv<0) {
        plane->sprite=fallingsprites[plane->colour==MOV_CYAN ? 0 : 1]
                                    [plane->inverted ? 1 : 0];
        plane->coll=fallingspritescoll[plane->inverted ? 1 : 0];
      }
      else {
        plane->sprite=planesprites[plane->colour==MOV_CYAN ? 0 : 1]
                                  [plane->inverted ? 1 : 0][plane->angle];
        plane->coll=planespritescoll[plane->inverted ? 1 : 0][plane->angle];
      }
  moveobject(plane,&x,&y);
  x=plane->x=max(0,min(x,MAX_X-plane->width));
  if (!planeiscomputer &&
      (plane->state==FLYING || plane->state==STALLED ||
       plane->state==WOUNDED || plane->state==WOUNDSTALL) &&
      endstatuses[playerindex]==NOTFINISHED)
    alertcomputer(plane);
  deletex(plane);
  insertx(plane,plane->nextx);
  if (plane->bomb_delay!=0)
    plane->bomb_delay--;
  if (plane->missile_delay!=0)
    plane->missile_delay--;
  if (plane->starburst_delay!=0)
    plane->starburst_delay--;
  if (!planeiscomputer && plane->athome && plane->state==FLYING)
    refuel(plane);
  if (y<MAX_Y && y>=0) {
    if (plane->state==FALLING || plane->state==WOUNDED ||
        plane->state==WOUNDSTALL)
      createsmoke(plane);
    usescreenbuf();
    drawmapobject(plane);
    return (planeisplayer || plane->state<FINISHED);
  }
  return FALSE;
}

void alertcomputer(object* obj)
{
  static int left_terr[MAX_PLAYERS]={0,1155,0,2089};
  static int right_terr[MAX_PLAYERS]={0,2088,1154,MAX_X};

  object* plane;
  object* attackee;
  int i,x=obj->x,colour=obj->owner->colour;
  for (plane=firstobject+1,i=1;plane->type==PLANE;plane++,i++)
    if (plane->owner->colour!=colour &&
        plane->update_func==updatecomputerplane &&
        (gamemode!=COMPUTER || (left_terr[i]<=x && right_terr[i]>=x))) {
      attackee=compnear[i];
      if (attackee==NULL || abs(x-plane->x)<abs(attackee->x-plane->x))
        compnear[i]=obj;
    }
}

void refuel(object* plane)
{
  usescreenbuf();
  if (topup(&plane->life,MAX_FUEL))
    displayfuelgauge(plane);
  if (topup(&plane->ammo,MAX_AMMO))
    displayammogauge(plane);
  if (topup(&plane->bombs,MAX_BOMBS))
    displaybombgauge(plane);
#if VERSION==7
  if (topup(&plane->missiles,MAX_MISSILES))
    displaymissilegauge(plane);
  if (topup(&plane->starbursts,MAX_STARBURSTS))
    displaystarburstgauge(plane);
#endif
}

bool topup(int* val,int max)
{
  bool redraw=FALSE;
  if (*val!=max) {
    if (max<20) {
      if (framecounter%20==0) {
        (*val)++;
        redraw=planeisplayer;
      }
    }
    else {
      *val+=max/100;
      redraw=planeisplayer;
    }
    *val=min(*val,max);
  }
  return redraw;
}

bool updatebullet(object* bullet)
{
  int x,y;
  deletex(bullet);
  bullet->life--;
  if (bullet->life==0) {
    deleteobject(bullet);
    return FALSE;
  }
  moveobject(bullet,&x,&y);
  if (x<0 || x>=MAX_X || y<=(int)ground[x] || y>=MAX_Y) {
    deleteobject(bullet);
    return FALSE;
  }
  insertx(bullet,bullet->nextx);
  bullet->sprite=pixelsprites[0];
  bullet->coll=pixelspritescoll;
  return TRUE;
}

void adjustfall(object* obj)
{
  obj->life--;
  if (obj->life==0) {
    if (obj->yv<0)
      if (obj->xv<0)
        obj->xv++;
      else
        if (obj->xv>0)
          obj->xv--;
    if (obj->yv>TERMINAL_VELOCITY)
      obj->yv--;
    obj->life=BOMBLIFE;
  }
}

bool updatebomb(object* bomb)
{
  int x,y;
  deletex(bomb);
  if (bomb->life<0) {
    deleteobject(bomb);
    bomb->state=FINISHED;
    usescreenbuf();
    drawmapobject(bomb);
    return FALSE;
  }
  adjustfall(bomb);
  if (bomb->yv<=0)
    initsound(bomb,SOUND_BOMB);
  moveobject(bomb,&x,&y);
  if (x<0 || x>=MAX_X || y<0) {
    deleteobject(bomb);
    stopsound(bomb);
    bomb->state=FINISHED;
    usescreenbuf();
    drawmapobject(bomb);
    return FALSE;
  }
  bomb->sprite=bombsprites[bomb->colour==MOV_CYAN ? 0 : 1][direction(bomb)];
  bomb->coll=bombspritescoll[direction(bomb)];
  insertx(bomb,bomb->nextx);
  if (y>=MAX_Y)
    return FALSE;
  usescreenbuf();
  drawmapobject(bomb);
  return TRUE;
}

int direction(object* obj)
{
  int xv=obj->xv,yv=obj->yv;
  if (yv>0)  { if (xv<0) return 3; if (xv==0) return 2; return 1; }
  if (yv==0) { if (xv<0) return 4; if (xv==0) return 6; return 0; }
               if (xv<0) return 5; if (xv==0) return 6; return 7;
}

bool updatemissile(object* missile)
{
  int x,y,angle;
  object* target;
  deletex(missile);
  if (missile->life<0) {
    deleteobject(missile);
    missile->state=FINISHED;
    usescreenbuf();
    drawmapobject(missile);
    return FALSE;
  }
  if (missile->state==FLYING) {
    target=missile->target;
    if (target!=missile->owner && (missile->life&1)==1) {
      if (target->target!=NULL)
        target=target->target;
      aim(missile,target->x,target->y,NULL,FALSE);
      angle=missile->angle=(missile->angle+missile->flaps)&(ANGLES-1);
      setvel(missile,missile->speed,angle);
    }
    moveobject(missile,&x,&y);
    missile->life--;
    if (missile->life==0 || y>=(MAX_Y*3)/2) {
      missile->state=FALLING;
      missile->life++;
    }
  }
  else {
    adjustfall(missile);
    missile->angle=(missile->angle+1)&(ANGLES-1);
    moveobject(missile,&x,&y);
  }
  if (y<0 || x<0 || x>=MAX_X) {
    deleteobject(missile);
    missile->state=FINISHED;
    usescreenbuf();
    drawmapobject(missile);
    return FALSE;
  }
  missile->sprite=missilesprites[missile->colour==MOV_CYAN ? 0 : 1]
                                [missile->angle];
  missile->coll=missilespritescoll[missile->angle];
  insertx(missile,missile->nextx);
  if (y>=MAX_Y)
    return FALSE;
  usescreenbuf();
  drawmapobject(missile);
  return TRUE;
}

bool updatestarburst(object* starburst)
{
  int x,y;
  deletex(starburst);
  if (starburst->life<0) {
    starburst->owner->target=NULL;
    deleteobject(starburst);
    return FALSE;
  }
  adjustfall(starburst);
  moveobject(starburst,&x,&y);
  if (y<=(int)ground[x] || x<0 || x>=MAX_X) {
    starburst->owner->target=NULL;
    deleteobject(starburst);
    return FALSE;
  }
  starburst->owner->target=starburst;
  starburst->sprite=starburstsprites[starburst->colour==MOV_CYAN ? 0 : 1]
                                    [starburst->life&1];
  starburst->coll=starburstspritescoll[starburst->life&1];
  insertx(starburst,starburst->nextx);
  return (y<MAX_Y);
}

bool updatetarget(object* target)
{
  int d2;
  object* at=firstobject;
  target->firing=FALSE;
  if (level!=0 && target->state==STANDING &&
      (at->state==FLYING || at->state==STALLED || at->state==WOUNDED ||
       at->state==WOUNDSTALL) &&
      target->colour!=at->colour && (level>1 || (framecounter&1)==1)) {
    d2=range(target->x,target->y,at->x,at->y);
    if (d2>0 && d2<targetrange2) {
      target->firing=TRUE;
      createbullet(target,at);
    }
  }
  target->hitcounter--;
  if (target->hitcounter<0)
    target->hitcounter=0;
  if (target->state==STANDING) {
    target->sprite=targetsprites[target->colour==MOV_CYAN ? 0 : 1]
                                [target->subtype];
    target->coll=targetspritescoll[target->subtype];
  }
  else {
    target->sprite=debrissprites[target->colour==MOV_CYAN ? 0 : 1];
    target->coll=debrisspritescoll;
  }
  return TRUE;
}

bool updatefrag(object* frag)
{
  int x,y,subtype=frag->subtype;
  deletex(frag);
  if (frag->life<0) {
    if (subtype!=FRAG_PERSON)
      stopsound(frag);
    deleteobject(frag);
    return FALSE;
  }
  frag->life--;
  if (frag->life==0) {
    if (frag->yv<0)
      if (frag->xv<0)
        frag->xv++;
      else
        if (frag->xv>0)
          frag->xv--;
    if (frag->yv>(subtype==FRAG_PERSON ? -minspeed : TERMINAL_VELOCITY))
      frag->yv--;
    frag->life=FRAGLIFE;
  }
  moveobject(frag,&x,&y);
  if (y<=(int)ground[x] || x<0 || x>=MAX_X) {
    if (subtype!=FRAG_PERSON)
      stopsound(frag);
    deleteobject(frag);
    return FALSE;
  }
  frag->hitcounter++;
#if VERSION==1
  frag->sprite=fragsprites[0][frag->subtype];
#else
  frag->sprite=fragsprites[frag->colour==MOV_CYAN ? 1 : 2][frag->subtype];
#endif
  frag->coll=fragspritescoll[frag->subtype];
  insertx(frag,frag->nextx);
  return y<MAX_Y;
}

bool updatesmoke(object* smoke)
{
  int state=smoke->owner->state;
  smoke->life--;
  if (smoke->life==0 || (state!=FALLING && state!=WOUNDED &&
                         state!=WOUNDSTALL && state!=CRASHED)) {
    deleteobject(smoke);
    return FALSE;
  }
  smoke->sprite=pixelsprites[smoke->colour==MOV_CYAN ? 1 : 2];
  smoke->coll=pixelspritescoll;
  return TRUE;
}

bool updateflock(object* flock)
{
  int x,y;
  deletex(flock);
  if (flock->life==-1) {
    usescreenbuf();
    drawmapobject(flock);
    deleteobject(flock);
    return FALSE;
  }
  flock->life--;
  if (flock->life==0) {
    flock->inverted=!flock->inverted;
    flock->life=FLOCKLIFE;
  }
  if (flock->x<MIN_FLOCK_X || flock->x>MAX_FLOCK_X)
    flock->xv=-flock->xv;
  moveobject(flock,&x,&y);
  insertx(flock,flock->nextx);
  flock->sprite=flocksprites[flock->inverted ? 1 : 0];
  flock->coll=flockspritescoll[flock->inverted ? 1 : 0];
  usescreenbuf();
  drawmapobject(flock);
  return TRUE;
}

bool updatebird(object* bird)
{
  int x,y;
  deletex(bird);
  if (bird->life==-1) {
    deleteobject(bird);
    return FALSE;
  }
  if (bird->life==-2) {
    bird->yv=-bird->yv;
    bird->xv=(framecounter&7)-4;
    bird->life=BIRDLIFE;
  }
  else {
    bird->life--;
    if (bird->life==0) {
      bird->inverted=!bird->inverted;
      bird->life=BIRDLIFE;
    }
  }
  moveobject(bird,&x,&y);
  insertx(bird,bird->nextx);
  bird->sprite=birdsprites[bird->inverted ? 1 : 0];
  bird->coll=birdspritescoll[bird->inverted ? 1 : 0];
  if (x<0 || x>=MAX_X || y<=(int)ground[x] || y>=MAX_Y) {
    bird->y-=bird->yv;
    bird->life=-2;
    return FALSE;
  }
  return TRUE;
}

bool updateox(object* ox)
{
  ox->sprite=oxsprites[ox->state==STANDING ? 0 : 1];
  ox->coll=oxspritescoll[ox->state==STANDING ? 0 : 1];
  return TRUE;
}

void endgame(int targetcolour)
{
  int wincolour;
  object* player;
  if (!truemultiplayer)
    wincolour=1;
  else
    if ((firstobject+1)->score==firstobject->score)
      wincolour=(MOV_CYAN+MOV_MAGENTA)-targetcolour;
    else
      wincolour=((firstobject+1)->score>firstobject->score) ? MOV_MAGENTA :
                                                              MOV_CYAN;
  for (player=firstobject;player->type==PLANE;player=player->next)
    if (endstatuses[player->index]==NOTFINISHED)
      if (player->colour==wincolour &&
          (player->lives>1 ||
           (player->lives>0 &&
            (player->state==FLYING || player->state==STALLED ||
             player->state==WOUNDED || player->state==WOUNDSTALL))))
        winner(player);
      else
        loser(player);
}

void winner(object* plane)
{
  int index=plane->index;
  endstatuses[index]=WINNER;
  if (index==playerindex) {
    endcounter=72;
    flyingintosunset=TRUE;
    plane->xv=plane->yv=plane->xvfrac=plane->yvfrac=0;
    plane->state=FLYING;
    plane->life=MAX_FUEL;
    plane->speed=MIN_SPEED;
  }
}

void loser(object* plane)
{
  int player=plane->index;
  endstatuses[player]=LOSER;
  if (player==playerindex) {
    setcolour(XOR_MAGENTA);
    poscurs(16,12);
    printstr("THE END");
    endcounter=20;
  }
}

void computerpilot(object* plane)
{
  object* target=compnear[planeindex];
  if (target!=NULL)
    attack(plane,target);
  else
    if (!plane->athome)
      cruise(plane);
  compnear[planeindex]=NULL;
}

void attack(object* plane,object* target)
{
  courseadjust=((framecounter&0x1f)<16 ? 16 : 0);
  if (target->speed!=0)
    aim(plane,target->x-(fcos(CLOSE,target->angle)>>8),
              target->y-(fsin(CLOSE,target->angle)>>8),target,FALSE);
  else
    aim(plane,target->x,target->y+4,target,FALSE);
}

void cruise(object* plane)
{
  int xhome;
  courseadjust=((framecounter&0x1f)<16 ? 16 : 0);
  xhome=originalplanes[planeindex].x;
  aim(plane,max(MAX_X/3,min(xhome,2*MAX_X/3))+courseadjust,
      CRUISEHEIGHT-(courseadjust>>1),NULL,FALSE);
}

void gohome(object* plane)
{
  object* base;
  if (plane->athome)
    return;
  base=&originalplanes[planeindex];
  courseadjust=((framecounter&0x1f)<16 ? 16 : 0);
  if (abs(plane->x-base->x)<HOMEDIST && abs(plane->y-base->y)<HOMEDIST) {
    if (planeisplayer) {
      createplayerplane(plane);
      initdisplay(TRUE);
    }
    else
      if (planeiscomputer)
        createcomputerplane(plane);
      else
        createplane(plane);
    return;
  }
  aim(plane,base->x,base->y,NULL,FALSE);
}

object tempplane;

int gunontarget(object* target)
{
  object tempbullet,temptarget;

  int bulletx,bullety,targetx,targety;
  int newspeed,newangle;
  int d2,i;
  memcpy(&tempbullet,&tempplane,sizeof(object));
  memcpy(&temptarget,target,sizeof(object));
  setvel(&tempbullet,tempbullet.speed+BULLETSPEED,tempbullet.angle);
  tempbullet.x+=PLANE_WIDTH/2;
  tempbullet.y-=PLANE_HEIGHT/2;
  newangle=temptarget.angle;
  newspeed=temptarget.speed;
  for (i=0;i<BULLETLIFE;i++) {
    moveobject(&tempbullet,&bulletx,&bullety);
#if VERSION!=2
    if ((temptarget.state==FLYING || temptarget.state==WOUNDED) &&
        temptarget.flaps!=0) {
      if (temptarget.inverted)
        newangle-=temptarget.flaps;
      else
        newangle+=temptarget.flaps;
      newangle=newangle&(ANGLES-1);
      setvel(&temptarget,newspeed,newangle);
    }
#endif
    moveobject(&temptarget,&targetx,&targety);
    d2=range(bulletx,bullety,targetx,targety);
    if (d2<0 || d2>NEAR)
      return BULLET_MISS;
    if (bulletx>=targetx && bulletx<=targetx+PLANE_WIDTH-1 &&
        bullety<=targety && bullety>=targety-(PLANE_HEIGHT-1))
      return (i>BULLETLIFE/3 ? BULLET_LONGRANGE : BULLET_SHORTRANGE);
  }
  return BULLET_MISS;
}

int lefttarget,righttarget;

void cleartargs(void)
{
  lefttarget=-2;
}

void inittargetcheck(int x)
{
  int i;
  lefttarget=-1;
  righttarget=0;
  for (i=0;i<MAX_AVOIDABLES;i++)
    if (targets[i]!=NULL && targets[i]->x>=x-(32+maxspeed)) {
      lefttarget=i;
      break;
    }
  if (lefttarget==-1)
    return;
  for (;i<MAX_AVOIDABLES;i++)
    if (targets[i]==NULL || targets[i]->x>=x+(32+maxspeed))
      break;
  righttarget=i-1;
}

bool doomed(object* plane,int x,int y,int alt)
{
  object* target;
  int i,xt,yt;
  if (alt>50)
    return FALSE;
#if VERSION==2
  if (alt<20)
#else
  if (alt<22)
#endif
    return TRUE;
  if (lefttarget==-2)
    inittargetcheck(plane->x);
  for (i=lefttarget;i<=righttarget;i++) {
    target=targets[i];
    xt=target->x;
    if (xt>=x-32) {
      if (xt>x+32)
        return FALSE;
      yt=target->y+(target->state==STANDING ? 16 : 8);
      if (y<=yt)
        return TRUE;
    }
  }
  return FALSE;
}

int range(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
#if VERSION!=2
  y+=(y>>1);
#endif
  if (x<125 && y<125)
    return x*x+y*y;
  if (x<y) { t=x; x=y; y=t; }
  return -((x*7+(y<<2))>>3);
}

void aim(object* plane,int destx,int desty,object* target,bool longway)
{
  static int autoflaps[3]={0,-1,1};
  static int autorange[3];
  static bool autodoomed[3];
  static int autoalt[3];

  int x,y,distx,disty,i,newx,newy,n,newangle,newspeed,r,minrange;
  if ((plane->state==STALLED || plane->state==WOUNDSTALL) &&
      plane->angle!=3*ANGLES/4) {
    plane->flaps=-1;
#if VERSION!=2
    plane->accel=MAX_THROTTLE;
#endif
    return;
  }
  x=plane->x;
  y=plane->y;
  distx=x-destx;
  if (abs(distx)>LONGWAY) {
    if (plane->xv!=0 && (distx<0)==(plane->xv<0)) {
      if (plane->hitcounter==0)
        plane->hitcounter=(y>CRUISEHEIGHT ? 2 : 1);
      aim(plane,x,plane->hitcounter==1 ? y+25 : y-25,NULL,TRUE);
      return;
    }
    plane->hitcounter=0;
    y+=100;
    aim(plane,(distx<0 ? 150 : -150)+x,min(CRUISEHEIGHT-courseadjust,y),NULL,
        TRUE);
    return;
  }
  if (!longway)
    plane->hitcounter=0;
  if (plane->speed!=0) {
    disty=y-desty;
    if (disty!=0 && abs(disty)<6)
      plane->y=(disty<0 ? ++y : --y);
    else
      if (distx!=0 && abs(distx)<6)
        plane->x=(distx<0 ? ++x : --x);
  }
#if VERSION==2
  inittargetcheck(x);
#else
  cleartargs();
#endif
  memcpy(&tempplane,plane,sizeof(object));
  if (tempplane.type==PLANE)
    newspeed=max(minspeed,min(tempplane.speed+1,maxspeed));
  else
    newspeed=max(minspeed,tempplane.speed+1);
  for (i=0;i<3;i++) {
    newangle=(tempplane.angle+(tempplane.inverted ? -autoflaps[i] :
                                                     autoflaps[i]))&(ANGLES-1);
    setvel(&tempplane,newspeed,newangle);
    moveobject(&tempplane,&newx,&newy);
    autorange[i]=range(newx,newy,destx,desty);
    autoalt[i]=newy-originalground[newx+8];
    autodoomed[i]=doomed(plane,newx,newy,autoalt[i]);
    memcpy(&tempplane,plane,sizeof(object));
  }
  if (target!=NULL) {
    i=gunontarget(target);
    if (i!=BULLET_MISS)
#if VERSION!=2
      if (plane->missiles!=0 && i==BULLET_LONGRANGE)
        plane->missile_target=(target->athome ? plane : target);
      else
#endif
        plane->firing=TRUE;
  }
  minrange=32767;
  for (i=0;i<3;i++) {
    r=autorange[i];
    if (r>=0 && r<minrange && !autodoomed[i]) {
      minrange=r;
      n=i;
    }
  }
  if (minrange==32767) {
    minrange=-32767;
    for (i=0;i<3;i++) {
      r=autorange[i];
      if (r<0 && r>minrange && !autodoomed[i]) {
        minrange=r;
        n=i;
      }
    }
  }
  if (plane->speed<minspeed)
    plane->accel=MAX_THROTTLE;
  if (minrange!=-32767) {
    if (plane->accel<MAX_THROTTLE)
      plane->accel++;
  }
  else {
    if (plane->accel!=0)
      plane->accel--;
    n=0;
    disty=autoalt[0];
    if (autoalt[1]>disty) {
      disty=autoalt[1];
      n=1;
    }
    if (autoalt[2]>disty)
      n=2;
  }
  plane->flaps=autoflaps[n];
  if (plane->type==PLANE && plane->flaps==0 && plane->speed!=0)
    plane->inverted=(plane->xv<0);
}

void crashplane(object* plane)
{
  object* origplane;
  if (plane->xv<0)
    plane->angle=(plane->angle+2)&(ANGLES-1);
  else
    plane->angle=(plane->angle-2)&(ANGLES-1);
  plane->state=(plane->state>=GHOST ? GHOSTCRASHED : CRASHED);
  plane->athome=FALSE;
  plane->xv=plane->yv=plane->xvfrac=plane->yvfrac=plane->speed=0;
#if VERSION!=7
  plane->hitcounter=MAXCRCOUNT;
#else
  origplane=&originalplanes[plane->index];
  plane->hitcounter=((abs(origplane->x-plane->x)<SAFERESET) &&
                     (abs(origplane->y-plane->y)<SAFERESET)) ?
                      (MAXCRCOUNT<<1) : MAXCRCOUNT;
#endif
}

void hitplane(object* plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->hitcounter=FALLCOUNT;
  plane->state=FALLING;
  plane->athome=FALSE;
}

void stallplane(object* plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->xv=0;
  plane->yv=0;
  plane->inverted=FALSE;
  plane->angle=7*ANGLES/8;
  plane->speed=0;
  plane->hitcounter=STALLCOUNT;
  plane->state=(plane->state>=GHOST) ? GHOSTSTALLED :
                                       ((plane->state==WOUNDED) ? WOUNDSTALL :
                                                                  STALLED);
  plane->athome=FALSE;
}

void insertx(object* ins,object* list)
{
  if (ins->x<list->x)
    do
      list=list->prevx;
    while (ins->x<list->x);
  else {
    while (ins->x>=list->x)
      list=list->nextx;
    list=list->prevx;
  }
  ins->nextx=list->nextx;
  ins->prevx=list;
  list->nextx->prevx=ins;
  list->nextx=ins;
}

void deletex(object* del)
{
  del->nextx->prevx=del->prevx;
  del->prevx->nextx=del->nextx;
}

void inittonetable(void)
{
  tonetable* tt;
  int i;
  for (i=0,tt=tonetab;i<SOUNDSIZE-1;i++,tt++)
    tt->next=tt+1;
  tt->next=NULL;
  firsttone=NULL;
  freetone=tonetab;
}

void setsound(int type,int param,object* obj)
{
  if (type<soundtype) {
    soundtype=type;
    soundparam=param;
    soundobj=obj;
  }
  else
    if (type==soundtype && param<soundparam) {
      soundparam=param;
      soundobj=obj;
    }
}

void updatesound(void)
{
  tonetable* tt;
  for (tt=firsttone;tt!=NULL;tt=tt->next)
    tt->tone+=tt->change*soundticks;
  soundticks=0;
  titleflag=FALSE;
  switch (soundtype) {
    case SOUND_OFF:
    case SOUND_NONE:
    default:
      soundoff();
      lastobj=NULL;
      soundadjfunc=NULL;
      break;
    case SOUND_PLANE:
      if (soundparam==0)
        tone(0xf000);
      else
        tone(soundparam*0x1000+0xd000);
      lastobj=NULL;
      soundadjfunc=NULL;
      break;
    case SOUND_BOMB:
      if (soundobj==lastobj)
        break;
      soundadjfunc=soundadjslide;
      lastobj=soundobj;
      soundadjslide();
      break;
    case SOUND_FALLING:
      if (soundobj==lastobj)
        break;
      soundadjfunc=soundadjslide;
      lastobj=soundobj;
      soundadjslide();
      break;
    case SOUND_HIT:
      tone(soundrand(2)!=0 ? 0x9000 : 0xf000);
      lastobj=NULL;
      soundadjfunc=NULL;
      break;
    case SOUND_EXPLOSION:
      tone(expltone);
      soundadjfunc=NULL;
      lastobj=NULL;
      break;
    case SOUND_FIRING:
      tone(0x1000);
      soundadjfunc=soundadjshot;
      lastobj=NULL;
      break;
    case SOUND_TITLE:
      titleline=0;
      titleplace=0;
      titlenote();
      soundadjfunc=NULL;
      lastobj=NULL;
      titleflag=TRUE;
  }
  soundparam=0x7fff;
  soundtype=SOUND_NONE;
}

void soundadjust(void)
{
  soundticks++;
  if (lastfreq!=0 && soundadjfunc!=NULL)
    soundadjfunc();
  if (numfrags!=0) {
    explticks--;
    if (explticks<0)
      explnote();
  }
  if (titleflag) {
    titleticks--;
    if (titleticks<0)
      titlenote();
  }
}

void soundadjslide(void)
{
  tonetable* tt=lastobj->tonetab;
/* #if VERSION==7 */
  if (tt==NULL)
    return;
/* #endif */
  tone(tt->tone+tt->change*soundticks);
}

void soundadjshot(void)
{
  static unsigned int savefreq=0;
  if (lastfreq==0xf000)
    tone(savefreq);
  else {
    savefreq=lastfreq;
    tone(0xf000);
  }
}

void explnote(void)
{
  tuneline=explline;
  tuneplace=explplace;
  tune=sopwith2tune;
  octavemultiplier=exploctavemul;
  playnote();
  explline=tuneline;
  explplace=tuneplace;
  expltone=tunefreq;
  explticks+=tunedur;
  exploctavemul=octavemultiplier;
}

void titlenote(void)
{
  tuneline=titleline;
  tuneplace=titleplace;
  tune=sopwith2tune;
  octavemultiplier=titleoctavemul;
  playnote();
  titleline=tuneline;
  titleplace=tuneplace;
  titletone=tunefreq;
  titleticks+=tunedur;
  titleoctavemul=octavemultiplier;
  soundoff();
  tone(titletone);
}

void initsound(object* obj,int type)
{
  tonetable* tt;
  if (obj->tonetab!=NULL)
    return;
  if (obj->type==EXPLOSION) {
    numfrags++;
    if (numfrags==1) {
      explplace=explline=0;
      explnote();
    }
    obj->tonetab=(tonetable *)1; /* Bad style */
    return;
  }
  tt=alloctone();
  if (tt!=NULL) {
    switch (type) {
      case SOUND_BOMB:
        tt->tone=0x300;
        tt->change=8;
        break;
      case SOUND_FALLING:
        tt->tone=0x1200;
        tt->change=-8;
        break;
    }
    obj->tonetab=tt;
  }
}

tonetable* alloctone(void)
{
  tonetable* tt;
  if (freetone==NULL)
    return NULL;
  tt=freetone;
  freetone=tt->next;
  tt->next=firsttone;
  tt->prev=NULL;
  if (firsttone!=NULL)
    firsttone->prev=tt;
  firsttone=tt;
  return firsttone;
}

void dealloctone(tonetable* tt)
{
  tonetable* o=tt->prev;
  if (o!=NULL)
    o->next=tt->next;
  else
    firsttone=tt->next;
  o=tt->next;
  if (o!=NULL)
    o->prev=tt->prev;
  tt->next=freetone;
  freetone=tt;
}

void stopsound(object* obj)
{
  tonetable* tt=obj->tonetab;
  if (tt==NULL)
    return;
  if (obj->type==EXPLOSION)
    numfrags--;
  else
    dealloctone(tt);
  obj->tonetab=NULL;
}

void tone(unsigned int freq)
{
  if (soundflag && lastfreq!=freq) {
    if (lastfreq==0)
      outportb(PORT_TIMERC,0xb6);
    outportb(PORT_TIMER2,freq);
    outportb(PORT_TIMER2,freq>>8);
    if (lastfreq==0)
      outportb(PORT_SPKR,inportb(PORT_SPKR)|3);
    lastfreq=freq;
  }
}

void soundoff(void)
{
  if (lastfreq!=0) {
    outportb(PORT_SPKR,inportb(PORT_SPKR)&0xfc);
    lastfreq=0;
  }
}

int soundrand(int f)
{
  static int seed[50]={
    0x90b9,0xbcfb,0x6564,0x3313,0x3190,0xa980,0xbcf0,0x6f97,0x37f4,0x064b,
    0x9fd8,0x595b,0x1eee,0x820c,0x4201,0x651e,0x848e,0x15d5,0x1de7,0x1585,
    0xa850,0x213b,0x3953,0x1eb0,0x97a7,0x35dd,0xaf2f,0x1629,0xbe9b,0x243f,
    0x847d,0x313a,0x3295,0xbc11,0x6e6d,0x3398,0xad43,0x51ce,0x8f95,0x507e,
    0x499e,0x3bc1,0x5243,0x2017,0x9510,0x9865,0x65f6,0x6b56,0x36b9,0x5026};
  static int i=0;
  if (i>=50)
    i=0;
  return seed[i++]%f;
}

/* The array durbuf ought to be dynamically allocated. However, sometimes this
   function is called via an interrupt, and sometimes on these occasions the
   interrupt will have occurred during some sort of system call. Sometimes when
   this happens the stack segment will be different from this program's stack
   segment, so SS will not be equal to DS, which means that if durbuf[] is
   dynamically allocated (on the stack, i.e. in SS) and referenced through DS,
   memory will be corrupted. You have no idea how long this bug took to find.
   I can't think of a better way to fix it. */

void playnote(void)
{
  static int majorscale[7]={0,2,3,5,7,8,10};
  static int notefreq[12]={440,466,494,523,554,587,622,659,698,740,784,831};
  static char durbuf[5];

  int sharpen=0,duri=0,dotdur=2,noteoctave=0x100,semitone,duration,freq;
  char tunechar,noteletter;
  while (TRUE) {
    if (tuneline==0 && tuneplace==0)
      octavemultiplier=0x100;
    tunechar=toupper(tune[tuneline][tuneplace++]);
    if (tunechar==0) {
      tuneplace=0;
      tuneline++;
      tunechar=tune[tuneline][tuneplace];
      if (tunechar==0)
        tuneline=0;
    }
    else {
      if (tunechar=='/')
        break;
      if (isalpha(tunechar)) {
        semitone=majorscale[tunechar-'A'];
        noteletter=tunechar;
      }
      else
        switch(tunechar) {
          case '>':
            octavemultiplier<<=1;
            break;
          case '<':
            octavemultiplier>>=1;
            break;
          case '+':
            sharpen++;
            break;
          case '-':
            sharpen--;
            break;
          case '.':
            dotdur=3;
            break;
          default:
            if (isdigit(tunechar))
              durbuf[duri++]=tunechar;
        }
    }
  }
  durbuf[duri]=0;
  duration=atoi(durbuf);
  if (duration<=0)
    duration=4;
  if (noteletter=='R')
    freq=0x7d00;
  else {
    semitone+=sharpen;
    while (semitone<0) {
      semitone+=12;
      noteoctave>>=1;
    }
    while (semitone>=12) {
      semitone-=12;
      noteoctave<<=1;
    }
    freq=(short)(((long)notefreq[semitone]*(long)octavemultiplier*
                  (long)noteoctave)>>16);
  }
  tunefreq=(short)(1193181L/freq);
  tunedur=((dotdur*TEMPO)/(duration*60))>>1;
}

void initrand(void)
{
  while (randv==0) {
    outportb(PORT_TIMERC,0);
    randv=inportb(PORT_TIMER0);
    randv|=(inportb(PORT_TIMER0)<<8);
  }
}

void restart(void)
{
  int sc,i,timertickstart;
  object* plane;
  if (endstatuses[playerindex]==WINNER) {
    plane=&objects[playerindex];
    sc=0;
    while (plane->lives-->0) {
      sc+=25;
      plane->score+=sc;
      usescreenbuf();
      displayplanesgauge(plane);
      displayscore(plane);
      timertickstart=timertick;
      do {
        processtimerticks();
        flushkeybuf(); /* Jornand de Buisonje <jdbuiso@cs.vu.nl> is the only
                       person who would ever bother to fill up the keyboard
                       buffer here, so this line is only here because of him.*/
      } while (timertick-timertickstart<5);
    }
    if (level<=5)
      level++;
    savescore=plane->score;
  }
  else
    level=savescore=0;
  inittonetable();
  copyground();
  initobjects();
  createplayerplane(NULL);
  for (i=0;i<3;i++)
    createcomputerplane(NULL);
  createtargets();
  initdisplay(FALSE);
  createflocks();
  createoxen();
  initdifficulty();
  restarting=FALSE;
}

void initdifficulty(void)
{
  minspeed=MIN_SPEED+level;
  maxspeed=MAX_SPEED+level;
  targetrange2=40+10*min(level,6);
  targetrange2*=targetrange2;
}

void clearwin(void)
{
  int i;
  for (i=20;i<24;i++) {
    poscurs(0,i);
    clearline();
  }
  poscurs(0,20);
}

void getcontrol(void)
{
  int k;
  clearwin();
  printstr("Key: 1 - Joystick with IBM Keyboard\r\n");
  printstr("     2 - Joystick with non-IBM Keyboard\r\n");
  printstr("     3 - IBM Keyboard only\r\n");
  printstr("     4 - Non-IBM keyboard only\r\n");
  while (TRUE) {
    processtimerticks();
    if (testbreak())
      finish(NULL);
    k=inkeys();
    if (k>='1' && k<='4')
      break;
  }
  joystick=(k<='2');
  ibmkeyboard=(k=='1' || k=='3');
}

int getgamemode(void)
{
  clearwin();
  printstr("Key: S - single player\r\n");
  printstr("     C - single player against computer\r\n");
  printstr("     M - multiple players on network\r\n");
  printstr("     A - 2 players on asynchronous line");
  while (TRUE) {
    processtimerticks();
    if (testbreak())
      finish(NULL);
    switch (toupper(inkeys())) {
      case 'S':
        return SINGLE;
      case 'M':
        return MULTIPLE;
      case 'C':
        return COMPUTER;
      case 'A':
        return ASYNCH;
    }
  }
}

int getgamenumber(void)
{
  int n;
  clearwin();
  printstr("         Key a game number");
  while (TRUE) {
    processtimerticks();
    if (testbreak())
      finish(NULL);
    n=inkeys()-'0';
    if (n>=0 && n<=7)
      return n;
  }
}

bool testbreak(void)
{
  return breakf;
}

void copyground(void)
{
  memcpy(ground,originalground,sizeof(ground_t)*MAX_X);
}

void initdisplay(bool drawnground)
{
  object* player;
  object ghostobj;
  splatox=FALSE;
  oxsplatted=FALSE;
  if (!drawnground) {
    clearauxbuf();
    useauxbuf();
#if VERSION==1
    drawinitialground();
#endif
    drawmapground();
#if VERSION!=7
    if (!notitlesflag) {
      setsound(SOUND_OFF,0,NULL);
      updatesound();
    }
#endif
    ghost=FALSE;
  }
  farmemset(MK_FP(SCR_SEG,0),0,0x1000);
  farmemset(MK_FP(SCR_SEG,SCR_INTERLACE),0,0x1000);
  farmemmove(MK_FP(SCR_SEG,0x1000),MK_FP(_DS,(int)(&auxbuf[0])),0x1000);
  farmemmove(MK_FP(SCR_SEG,0x3000),MK_FP(_DS,(int)(&auxbuf[0])+0x1000),0x1000);
  usescreenbuf();
  drawmaptargets();
  initscores();
  player=&objects[playerindex];
  if (ghost) {
#if VERSION==2
    poscurs(16,24);
    setcolour(player->colour);
    printstr("\2"); /* :-) */
#else
    ghostobj.type=DUMMYTYPE;
    ghostobj.height=ghostobj.width=8;
    ghostobj.sprite=ghostsprites[player->colour==MOV_CYAN ? 0 : 1];
    drawobject(GHOSTX,12,&ghostobj);
#endif
  }
  else {
    displayfuelgauge(player);
    displaybombgauge(player);
#if VERSION==7
    displaymissilegauge(player);
    displaystarburstgauge(player);
#endif
    displayammogauge(player);
    displayplanesgauge(player);
  }
  groundtobedrawn=TRUE;
}

void initscores(void)
{
  if (savescore!=0) {
    objects[0].score=savescore;
    savescore=0;
  }
  displayscore(&objects[0]);
  if (truemultiplayer)
    displayscore(&objects[1]);
}

void displayplanesgauge(object* player)
{
  displaygauge(PLANESGAUGEX,player->lives,maxlives,player->colour);
}

void displayfuelgauge(object* player)
{
  displaygauge(FUELGAUGEX,player->life>>4,MAX_FUEL>>4,player->colour);
}

void displaybombgauge(object* player)
{
  displaygauge(BOMBGAUGEX,player->bombs,MAX_BOMBS,
               MOV_CYAN+MOV_MAGENTA-player->colour);
}

void displayammogauge(object* player)
{
  displaygauge(AMMOGAUGEX,player->ammo,MAX_AMMO,MOV_WHITE);
}

void displaymissilegauge(object* player)
{
  displaygauge(MISSILEGAUGEX,player->missiles,MAX_MISSILES,player->colour);
}

void displaystarburstgauge(object* player)
{
  displaygauge(STARBURSTGAUGEX,player->starbursts,MAX_STARBURSTS,
               player->colour==MOV_CYAN ? MOV_MAGENTA : MOV_CYAN);
}

void displaygauge(int x,int h,int hmax,int colour)
{
  int y;
  if (!ghost) {
    h=min(h*GAUGE_HEIGHT/hmax-1,GAUGE_HEIGHT-1);
    for (y=0;y<=h;y++)
      putpixel(x,y,colour);
    for (;y<GAUGE_HEIGHT;y++)
      putpixel(x,y,MOV_BLACK);
  }
}

void drawmapground(void)
{
  int x,y,dx=0,maxy,sx=SCR_CENTER;
  maxy=y=0;
  for (x=0;x<MAX_X;x++) {
    maxy=max(ground[x],maxy);
    dx++;
    if (dx==MAPDIVX) {
      maxy/=MAPDIVY;
      if (maxy==y)
        putpixel(sx,maxy,MOV_GREEN);
      else
        if (maxy>y)
          for (y++;y<=maxy;y++)
            putpixel(sx,y,MOV_GREEN);
        else
          for (y--;y>=maxy;y--)
            putpixel(sx,y,MOV_GREEN);
      y=maxy;
      putpixel(sx,0,MOV_BROWN);
      sx++;
      dx=maxy=0;
    }
  }
  for (y=0;y<=MAX_Y/MAPDIVY;y++) {
    putpixel(SCR_CENTER,y,MOV_BROWN);
    putpixel(sx,y,MOV_BROWN);
  }
  for (x=0;x<SCR_WIDTH;x++)
    putpixel(x,SCR_SCROLLBOT+2,MOV_GREEN);
}

void drawmaptargets(void)
{
  int i;
  object* target;
  mapobject* mapobj;
  for (i=0,target=objects,mapobj=mapobjects;i<MAX_OBJECTS;i++,target++,
                                                          mapobj++) {
    target->deleteflag=FALSE;
    target->drawflag=FALSE;
    mapobj->colour=0;
  }
  for (i=0;i<MAX_TARGETS;i++) {
    target=targets[i];
    if (target!=NULL && target->state!=FINISHED)
      drawmapobject(target);
  }
}

void useauxbuf(void)
{
  scrp=(unsigned char far*)MK_FP(_DS,(int)(&auxbuf[0])-0x1000);
  interlacediff=0x1000;
}

void usescreenbuf(void)
{
  scrp=(unsigned char far*)MK_FP(SCR_SEG,0);
  interlacediff=SCR_INTERLACE;
}

void clearauxbuf(void)
{
  memset(auxbuf,0,0x2000);
}

void initobjects(void)
{
  int i;
  object* obj;
  objleft.nextx=objleft.next=&objright;
  objright.prevx=objright.prev=&objleft;
  objleft.x=-32767;
  objright.x=32767;
  lastobject=firstobject=firstdeleted=lastdeleted=NULL;
  nextfreeobject=obj=objects;
  for (i=0;i<MAX_OBJECTS;i++) {
    obj->next=obj+1;
    (obj++)->index=i;
  }
  (obj-1)->next=NULL;
}

void createremoteplane(int state)
{
  object* plane=createplane(NULL);
  plane->display_func=displayremoteplane;
  plane->update_func=updateremoteplane;
  plane->colour=((plane->index&1)==0 ? MOV_CYAN : MOV_MAGENTA);
  plane->owner=plane;
  plane->state=state;
  memcpy(&originalplanes[plane->index],plane,sizeof(object));
}

void createcomputerplane(object* pl)
{
  object* plane=createplane(pl);
  if (pl==NULL) {
    plane->display_func=displaycomputerplane;
    plane->update_func=updatecomputerplane;
    plane->colour=MOV_MAGENTA;
    if (!multiplayer)
      plane->owner=&objects[1];
    else
      if (plane->index==1)
        plane->owner=plane;
      else
        plane->owner=plane-2;
    memcpy(&originalplanes[plane->index],plane,sizeof(object));
  }
  if (gamemode==SINGLE || gamemode==NOVICE) {
    plane->state=FINISHED;
    deletex(plane);
  }
}

void createplayerplane(object* player)
{
  object* plane=createplane(player);
  if (player==NULL) {
    plane->display_func=displayplayerplane;
    plane->update_func=updateplayerplane;
    plane->colour=((plane->index&1)==0 ? MOV_CYAN : MOV_MAGENTA);
    plane->owner=plane;
    memcpy(&originalplanes[plane->index],plane,sizeof(object));
    endcounter=1;
    flyingintosunset=FALSE;
  }
  screenleft=plane->x-SCR_CENTER;
  flushkeybuf();
}

object* createplane(object* pl)
{
#if VERSION==1
  static int runwayx[MAX_PLAYERS*2]={1270,588,1330,1360,1630,1660,2464,1720};
#else
  static int runwayx[MAX_PLAYERS*2]={1270,588,1330,1360,1630,1660,2456,1720};
#endif
  static bool runwayleft[MAX_PLAYERS*2]={FALSE,FALSE,FALSE,FALSE,
                                          TRUE,TRUE,TRUE,TRUE};
  static int singlerunways[2]={0,7};
  static int computerrunways[4]={0,7,1,6};
  static int multiplerunways[8]={0,7,3,4,2,5,1,6};

  int x,y,left,right,runway;
  object* plane=(pl!=NULL ? pl : allocobj());
  switch (gamemode) {
    case SINGLE:
    case NOVICE:
      runway=singlerunways[plane->index];
      break;
    case MULTIPLE:
    case ASYNCH:
      runway=multiplerunways[plane->index];
      break;
    case COMPUTER:
      runway=computerrunways[plane->index];
  }
  plane->type=PLANE;
  left=plane->x=runwayx[runway];
  right=left+RUNWAY_LENGTH;
  y=0;
  for (x=left;x<=right;x++)
    y=max(ground[x],y);
  plane->y=y+13;
  plane->bomb_delay=0;
  plane->hitcounter=0;
  plane->accel=0;
  plane->flaps=0;
  plane->speed=0;
  plane->xfrac=0;
  plane->yfrac=0;
  plane->missile_delay=0;
  plane->starburst_delay=0;
  setvel(plane,0,0);
  plane->inverted=runwayleft[runway];
  plane->angle=(plane->inverted ? ANGLES/2 : 0);
  plane->target=NULL;
  plane->firing=FALSE;
  plane->missile_target=NULL;
  plane->bombing=FALSE;
  plane->starburst_firing=FALSE;
  plane->goinghome=FALSE;
  plane->width=PLANE_WIDTH;
  plane->height=PLANE_HEIGHT;
  plane->athome=TRUE;
  if (pl==NULL || plane->state==CRASHED || plane->state==GHOSTCRASHED) {
    plane->ammo=MAX_AMMO;
    plane->bombs=MAX_BOMBS;
    plane->missiles=MAX_MISSILES;
    plane->starbursts=MAX_STARBURSTS;
    plane->life=MAX_FUEL;
  }
  if (pl==NULL) {
    plane->score=0;
    plane->lives=maxlives;
    endstatuses[plane->index]=NOTFINISHED;
    compnear[plane->index]=NULL;
    insertx(plane,&objleft);
  }
  else {
    deletex(plane);
    insertx(plane,plane->nextx);
  }
  if (multiplayer && plane->lives<=0) {
    plane->state=GHOST;
    if (plane->index==playerindex)
      ghost=TRUE;
  }
  else
    plane->state=FLYING;
  return plane;
}

void createbullet(object* obj,object* target)
{
  object* bullet;
  int xv,yv,r,bspeed,x,y;
  if (target==NULL && !planeiscomputer && obj->ammo==0)
    return;
  bullet=allocobj();
  if (bullet==NULL)
    return;
  if (gamemode!=NOVICE)
    obj->ammo--;
  bspeed=level+BULLETSPEED;
  if (target!=NULL) {
    x=target->x+(target->xv<<2);
    y=target->y+(target->yv<<2);
    xv=x-obj->x;
    yv=y-obj->y;
    r=range2(x,y,obj->x,obj->y);
    if (r<1) {
      deleteobject(bullet);
      return;
    }
    bullet->xv=(xv*bspeed)/r;
    bullet->yv=(yv*bspeed)/r;
    bullet->xvfrac=bullet->yvfrac=0;
  }
  else
    setvel(bullet,obj->speed+bspeed,obj->angle);
  bullet->type=SHOT;
  bullet->x=obj->x+PLANE_WIDTH/2;
  bullet->y=obj->y-PLANE_HEIGHT/2;
  bullet->xfrac=obj->xfrac;
  bullet->yfrac=obj->yfrac;
  bullet->life=BULLETLIFE;
  bullet->owner=obj;
  bullet->colour=obj->colour;
  bullet->height=bullet->width=1;
  bullet->display_func=NULL;
  bullet->update_func=updatebullet;
  bullet->speed=0;
  insertx(bullet,obj);
}

int range2(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
#if VERSION!=2
  y+=(y>>1);
#endif
  if (x>100 || y>100)
    return -1;
  if (x<y) { t=x; x=y; y=t; }
  return (x*7+(y<<2))>>3;
}

void createbomb(object* plane)
{
  object* bomb; /* Someone set up us the bomb! Sorry. */
  int angle;
  if ((!planeiscomputer && plane->bombs==0) || plane->bomb_delay!=0)
    return;
  bomb=allocobj();
  if (bomb==NULL)
    return;
  if (gamemode!=NOVICE)
    plane->bombs--;
  plane->bomb_delay=BOMB_DELAY;
  bomb->type=BOMB;
  bomb->state=FALLING;
  bomb->xv=plane->xv;
  bomb->yv=plane->yv;
#if VERSION==1
  setaux(bomb);
#endif
  if (plane->inverted)
    angle=(plane->angle+ANGLES/4)&(ANGLES-1);
  else
    angle=(plane->angle-ANGLES/4)&(ANGLES-1);
  bomb->x=plane->x+(fcos(10,angle)>>8)+4;
  bomb->y=plane->y+(fsin(10,angle)>>8)-4;
  bomb->xfrac=bomb->yfrac=bomb->xvfrac=bomb->yvfrac=0;
  bomb->life=BOMBLIFE;
  bomb->owner=plane;
  bomb->colour=plane->colour;
  bomb->width=bomb->height=8;
  bomb->display_func=displaybomb;
  bomb->update_func=updatebomb;
  insertx(bomb,plane);
}

void createmissile(object* plane)
{
  object* missile;
  int angle;
  if (plane->missile_delay!=0 || plane->missiles==0 || !missok)
    return;
  missile=allocobj();
  if (missile==NULL)
    return;
  if (gamemode!=NOVICE)
    plane->missiles--;
  plane->missile_delay=5;
  missile->type=MISSILE;
  missile->state=FLYING;
  missile->angle=angle=plane->angle;
  missile->x=plane->x+(fcos(1,angle)>>4)+4;
  missile->y=plane->y+(fsin(1,angle)>>4)-4;
  missile->xfrac=missile->yfrac=0;
  setvel(missile,missile->speed=maxspeed+(maxspeed>>1),missile->angle);
  missile->life=MISSILELIFE;
  missile->owner=plane;
  missile->colour=plane->colour;
  missile->height=missile->width=8;
  missile->display_func=displaymissile;
  missile->update_func=updatemissile;
  missile->target=plane->missile_target;
  missile->inverted=FALSE;
  missile->accel=missile->flaps=0;
  insertx(missile,plane);
}

void createstarburst(object* plane)
{
  object* starburst;
  int angle;
  if (plane->starburst_delay!=0 || plane->starbursts==0 || !missok)
    return;
  starburst=allocobj();
  if (starburst==NULL)
    return;
  starburst->starburst_delay=5;
  if (gamemode!=NOVICE)
    plane->starbursts--;
  starburst->type=STARBURST;
  starburst->state=FALLING;
  if (plane->inverted)
    angle=(plane->angle+(3*ANGLES/8))&(ANGLES-1);
  else
    angle=(plane->angle+(5*ANGLES/8))&(ANGLES-1);
  setvel(starburst,minspeed,angle);
  starburst->xv+=plane->xv;
  starburst->yv+=plane->yv;
  starburst->x=plane->x+(fcos(10,angle)>>10)+4;
  starburst->y=plane->y+(fsin(10,angle)>>10)-4;
  starburst->xfrac=starburst->yfrac=0;
  starburst->life=STARBURSTLIFE;
  starburst->owner=plane;
  starburst->colour=plane->colour;
  starburst->height=starburst->width=8;
  starburst->display_func=displaystarburst;
  starburst->update_func=updatestarburst;
  insertx(starburst,plane);
}

void createtargets(void)
{
  static int targetx[MAX_TARGETS]={
     191, 284, 409, 539, 685, 807, 934,1210,1240,1440,
    1550,1750,1780,2024,2159,2279,2390,2549,2678,2763};
  static int targettype[MAX_TARGETS]={
    TARGET_CHIMNEY,TARGET_TANK,TARGET_CHIMNEY,TARGET_CHIMNEY,TARGET_TANK,
    TARGET_FLAG,TARGET_CHIMNEY,TARGET_FUEL,TARGET_FLAG,TARGET_TANK,
    TARGET_TANK,TARGET_FLAG,TARGET_FUEL,TARGET_CHIMNEY,TARGET_CHIMNEY,
    TARGET_TANK,TARGET_TANK,TARGET_FLAG,TARGET_FLAG,TARGET_CHIMNEY};

  object* target;
  int i,minaltitude,maxaltitude,x,y,minx,maxx;
  if (!truemultiplayer) {
    numtargets[0]=0;
    numtargets[1]=MAX_TARGETS-3;
  }
  else
    numtargets[0]=numtargets[1]=MAX_TARGETS/2;
  for (i=0;i<MAX_TARGETS;i++) {
    targets[i]=target=allocobj();
    minx=target->x=targetx[i];
    maxx=minx+15;
    minaltitude=999;
    maxaltitude=0;
    for (x=minx;x<=maxx;x++) {
      maxaltitude=max(ground[x],maxaltitude);
      minaltitude=min(ground[x],minaltitude);
    }
    y=min((minaltitude+maxaltitude)>>1,MAX_Y-17);
    target->y=y+16;
    for (x=minx;x<=maxx;x++)
      ground[x]=y;
    target->xv=0;
    target->yv=0;
    target->xfrac=0;
    target->yfrac=0;
    target->xvfrac=0;
    target->yvfrac=0;
    target->angle=0;
    target->hitcounter=0;
    target->type=TARGET;
    target->state=STANDING;
    target->subtype=targettype[i];
    target->life=i;
    if (!truemultiplayer)
      target->owner=&objects[i>=MAX_TARGETS/2-3 && i<MAX_TARGETS/2 ? 0 : 1];
    else
      target->owner=&objects[i<MAX_TARGETS/2 ? 0 : 1];
    target->colour=target->owner->colour;
    target->height=target->width=16;
    target->display_func=displaytarget;
    target->update_func=updatetarget;
    insertx(target,&objleft);
  }
}

void createexplosion(object* obj)
{
  object* frag;
  int i,step,speed,x,y,xv,yv,colour,type,subtype;
  unsigned int randv1;
  bool pilotflag;
  x=obj->x+(obj->width>>1);
  y=obj->y+(obj->height>>1);
#if VERSION==2
  xv=obj->xv;
  yv=obj->yv;
#else
  xv=obj->xv>>2;
  yv=obj->yv>>2;
#endif
  colour=obj->colour;
  type=obj->type;
  if (type==TARGET && obj->subtype==TARGET_FUEL) {
    step=1;
#if VERSION==2
    speed=maxspeed;
#else
    speed=minspeed;
#endif
  }
  else {
    step=(type==PLANE ? 6 : 2);
#if VERSION==2
    speed=minspeed>>1;
#else
    speed=minspeed>>((randv&7)!=7 ? 1 : 0);
#endif
  }
  pilotflag=(type==PLANE && (obj->state==FLYING || obj->state==WOUNDED));
  for (i=1;i<=15;i+=step) {
    frag=allocobj();
    if (frag==NULL)
      return;
    frag->type=EXPLOSION;
    setvel(frag,speed,i);
    frag->xv+=xv;
    frag->yv+=yv;
    frag->x=frag->xv+x;
    frag->y=frag->yv+y;
#if VERSION==2
    randv1=(frag->y)*(frag->x)*randv*7491;
    subtype=frag->subtype=(int)(((unsigned long)((unsigned short)
                           ((unsigned long)randv1*(unsigned long)i))*8UL)>>16);
#else
    randv=(frag->y)*(frag->x)*randv+7491;
    if (randv==0)
      randv=74917777L;
    subtype=frag->subtype=(randv&0x1c0)>>6;
#endif
    frag->life=FRAGLIFE;
    if (pilotflag && (subtype==FRAG_PERSON || subtype==FRAG_DUST)) {
      pilotflag=FALSE;
      subtype=frag->subtype=FRAG_PERSON;
      frag->xv=xv;
      frag->yv=-minspeed;
    }
    frag->xfrac=frag->yfrac=frag->hitcounter=frag->speed=0;
    frag->owner=obj;
    frag->colour=colour;
    frag->height=frag->width=8;
    frag->display_func=displayfrag;
    frag->update_func=updatefrag;
    if (subtype!=FRAG_PERSON)
      initsound(frag,SOUND_EXPLOSION);
    insertx(frag,obj);
  }
}

void createsmoke(object* plane)
{
  object* smoke=allocobj();
  if (smoke==NULL)
    return;
  smoke->type=SMOKE;
  smoke->x=plane->x+8;
  smoke->y=plane->y-8;
  smoke->xv=plane->xv;
  smoke->yv=plane->yv;
  smoke->xfrac=smoke->yfrac=smoke->xvfrac=smoke->yvfrac=0;
  smoke->life=SMOKELIFE;
  smoke->owner=plane;
  smoke->height=smoke->width=1;
  smoke->display_func=NULL;
  smoke->update_func=updatesmoke;
  smoke->colour=plane->colour;
}

void createflocks(void)
{
  static int flockx[4]={MIN_FLOCK_X,MIN_FLOCK_X+1000,MAX_FLOCK_X-1000,
                        MAX_FLOCK_X};
  static int flocky[4]={MAX_Y-1,MAX_Y-1,MAX_Y-1,MAX_Y-1};
  static int flockxv[4]={2,2,-2,-2};

  object* flock;
  int i,bird;
  if (gamemode==NOVICE)
    return;
  for (i=0;i<MAX_FLOCKS;i++) {
    flock=allocobj();
    if (flock==NULL)
      return;
    flock->type=FLOCK;
    flock->state=FLYING;
    flock->x=flockx[i];
    flock->y=flocky[i];
    flock->xv=flockxv[i];
    flock->yv=flock->xfrac=flock->yfrac=flock->xvfrac=flock->yvfrac=0;
    flock->inverted=FALSE;
    flock->life=FLOCKLIFE;
    flock->owner=flock;
    flock->height=flock->width=16;
    flock->display_func=displayflock;
    flock->update_func=updateflock;
    flock->colour=MOV_TAN;
    insertx(flock,&objleft);
    for (bird=0;bird<STRAYBIRDS;bird++)
      createbird(flock,1);
  }
}

void createbird(object* flock,int birdno)
{
  static int fscatterx[8]={8,3,0,6,7,14,10,12};
  static int fscattery[8]={16,1,8,3,12,10,7,14};
  static int fscatterxv[8]={-2,2,-3,3,-1,1,0,0};
  static int fscatteryv[8]={-1,-2,-1,-2,-1,-2,-1,-2};

  object* bird=allocobj();
  if (bird==NULL)
    return;
  bird->type=BIRD;
  bird->x=flock->x+fscatterx[birdno];
  bird->y=flock->y-fscattery[birdno];
  bird->xv=fscatterxv[birdno];
  bird->yv=fscatteryv[birdno];
  bird->inverted=FALSE;
  bird->xfrac=bird->yfrac=bird->xvfrac=bird->yvfrac=0;
  bird->life=BIRDLIFE;
  bird->owner=flock;
  bird->height=2;
  bird->width=4;
  bird->display_func=displaybird;
  bird->update_func=updatebird;
  bird->colour=flock->colour;
  insertx(bird,flock);
}

void createoxen(void)
{
  static int oxx[2]={1376,1608};
  static int oxy[2]={80,91};

  object* ox;
  int i;
  if (gamemode==NOVICE) {
#if VERSION!=2
    for (i=0;i<MAX_OXEN;i++)
      targets[MAX_TARGETS+i]=NULL;
#endif
    return;
  }
  for (i=0;i<MAX_OXEN;i++) {
    ox=allocobj();
#if VERSION!=2
    targets[MAX_TARGETS+i]=ox;
#endif
    if (ox==NULL)       /* Test for the NULL ox. */
      return;
    ox->type=OX;
    ox->state=STANDING;
    ox->x=oxx[i];
    ox->y=oxy[i];
    ox->xv=0;
    ox->yv=0;           /* This is a particularly amusing variable */
    ox->xvfrac=0;
    ox->yvfrac=0;
    ox->xfrac=0;
    ox->yfrac=0;
    ox->inverted=FALSE; /* But this one is just plain bizarre */
    ox->owner=ox;
    ox->height=16;
    ox->width=16;
    ox->display_func=NULL;
    ox->update_func=updateox;
    ox->colour=MOV_CYAN;
    insertx(ox,&objleft);
  }
}

void checkcollisions(void)
{
  int right,top,bottom,i,type;
  object** obj1;
  object** obj2;
  object* obj;
  object* test;
  object* highest;
  object* lowest;

  collptr=killptr=0;
  collxadj=2;
  collyadj=1;
  if ((framecounter&1)==1) {
    collxadj=-collxadj;
    collyadj=-collyadj;
  }
  useauxbuf();
  for (obj=objleft.nextx;obj!=&objright;obj=obj->nextx) {
    right=obj->x+obj->width-1;
    bottom=obj->y;
    top=bottom-(obj->height-1);
    for (test=obj->nextx;test->x<=right && test!=&objright;test=test->nextx)
      if (test->y>=top && test->y-test->height+1<=bottom &&
          killptr<(MAX_OBJECTS<<1)-1 && checkcollision(obj,test)) {
        if (obj->y<test->y) {
          highest=test;
          lowest=obj;
        }
        else {
          highest=obj;
          lowest=test;
        }
        collobj1[killptr]=highest;
        collobj2[killptr++]=lowest;
        collobj1[killptr]=lowest;
        collobj2[killptr++]=highest;
      }
    type=obj->type;
    if (((type==PLANE && obj->state!=FINISHED && obj->state!=WAITING &&
         obj->y<ground[obj->x+8]+24) ||
        ((type==BOMB || type==MISSILE) && obj->y<ground[obj->x+4]+12)) &&
         killptr<(MAX_OBJECTS<<1) && checkcrash(obj)) {
      collobj1[killptr]=obj;
      collobj2[killptr++]=NULL;
    }
  }
  for (i=0,obj1=collobj1,obj2=collobj2;i<killptr;i++,obj1++,obj2++)
    docollision(*obj1,*obj2);
  for (i=0,obj1=collobjs;i<collptr;i++,obj1++) {
    obj=*obj1;
    obj->xv=collxv[i];
    obj->yv=collyv[i];
  }
}

bool checkcollision(object* obj1,object* obj2)
{
  unsigned int l1,l2,t1,t2,width,height;
  unsigned int widthbytes1,widthbytes2;
  unsigned int y,temp,firstbyte1,firstbyte2,firstbyteinrow1,firstbyteinrow2;
  unsigned int currentbyte1,currentbyte2,lastbyte;
  unsigned char head,bitpos1,bitpos2;
  int type1=obj1->type,type2=obj2->type;
  if ((type1==PLANE && obj1->state>=FINISHED) ||
      (type2==PLANE && obj2->state>=FINISHED) ||
      (type1==EXPLOSION && type2==EXPLOSION))
    return FALSE;
  if (obj1->x > obj2->x) {
    l1 = 0;
    l2 = obj1->x-obj2->x;
  }
  else {
    l1 = obj2->x-obj1->x;
    l2 = 0;
  }
  if (obj1->y < obj2->y) {
    t1 = 0;
    t2 = obj2->y-obj1->y;
  }
  else {
    t1 = obj1->y-obj2->y;
    t2 = 0;
  }
  if (obj1->x+obj1->width < obj2->x+obj2->width) {
    width = obj1->width-l1;
  }
  else {
    width = obj2->width-l2;
  }
  if (obj1->y-obj1->height > obj2->y-obj2->height) {
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
    for (y=0;y<height;y++) {
      if ((((obj1->coll[currentbyte1] & head) >> bitpos1) &
           ((*(unsigned int *)(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return TRUE;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
  }
  else {
    firstbyte1 = currentbyte1;
    firstbyte2 = currentbyte2;
    for (y=0;y<height;y++) {
      if (((obj1->coll[currentbyte1] >> bitpos1) &
           ((*(unsigned int *)(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return TRUE;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
    firstbyte1++;
    if (bitpos1 > bitpos2) {
      bitpos2 = 8 - bitpos1 + bitpos2;
    }
    else {
      bitpos2 = bitpos2 - bitpos1;
      firstbyte2++;
    }
    firstbyteinrow1 = firstbyte1;
    firstbyteinrow2 = firstbyte2;
    lastbyte = firstbyte1+(temp=(width+bitpos1)>>3)-2;
    for (y=0;y<height;y++) {
      for (currentbyte1=firstbyteinrow1,currentbyte2=firstbyteinrow2;
           currentbyte1<=lastbyte;currentbyte1++,currentbyte2++)
        if ((obj1->coll[currentbyte1] &
             ((*(unsigned int *)(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
          return TRUE;
      firstbyteinrow1+=widthbytes1;
      firstbyteinrow2+=widthbytes2;
      lastbyte+=widthbytes1;
    }
    if (((l1+width)&7) == 0) {
      return FALSE;
    }
    head = (1<<(((l1+width-1)&7)+1)) - 1;
    currentbyte1 = firstbyte1+temp-1;
    currentbyte2 = firstbyte2+temp-1;
    for (y=0;y<height;y++) {
      if (((obj1->coll[currentbyte1] & head) &
           ((*(unsigned int *)(obj2->coll+currentbyte2)) >> bitpos2)) != 0)
        return TRUE;
      currentbyte1+=widthbytes1;
      currentbyte2+=widthbytes2;
    }
  }
  return FALSE;
}

bool checkcrash(object* obj)
{
  unsigned int byte, widthbytes;
  unsigned char bit;
  int y;
  widthbytes=(obj->width+7)>>3;
  for (byte=0;byte<widthbytes;byte++)
    for (bit=0;bit<8 && (byte<<3)+bit < obj->width;bit++) {
      y=obj->y-(int)ground[(byte<<3)+bit+obj->x];
      if (y<0)
        return TRUE;
      if (y>=obj->height)
        continue;
      if ((obj->coll[y*widthbytes+byte] & (1<<bit)) != 0)
        return TRUE;
    }
  return FALSE;
}

void docollision(object* obj1,object* obj2)
{
  int state,type2,i;
  type2=(obj2!=NULL ? obj2->type : GROUND);
  if ((type2==BIRD || type2==FLOCK) && obj1->type!=PLANE)
    return;
  switch (obj1->type) {
    case BOMB:
    case MISSILE:
      createexplosion(obj1);
      obj1->life=-1;
      if (obj2==NULL) /* No object - it hit the ground */
        crater(obj1);
      stopsound(obj1);
      return;
    case SHOT:
      obj1->life=1;
      return;
    case STARBURST:
      if (type2==MISSILE || type2==BOMB || obj2==NULL)
        obj1->life=1;
      return;
    case EXPLOSION:
#if VERSION==2
      if (obj2==NULL || obj1->subtype!=FRAG_PERSON ||
          (obj1->subtype==FRAG_PERSON && scorepenalty(type2,obj2,200))) {
#else
      if (obj2==NULL) {
#endif
        obj1->life=1;
        stopsound(obj1);
      }
      return;
    case TARGET:
      if (obj1->state!=STANDING)
        return;
      if (type2==EXPLOSION || type2==STARBURST)
        return;
      if (type2==SHOT) {
        obj1->hitcounter+=TARGETHITCOUNT;
#if VERSION!=7
        if (obj1->hitcounter<TARGETHITCOUNT*(level+1))
#else
        if (obj1->hitcounter<=TARGETHITCOUNT*(level+1))
#endif
          return;
      }
      obj1->state=FINISHED;
      createexplosion(obj1);
      usescreenbuf();
      drawmapobject(obj1);
      useauxbuf();
      scoretarget(obj1,obj1->subtype==TARGET_FUEL ? 200 : 100);
      numtargets[obj1->colour-1]--;
      if (numtargets[obj1->colour-1]==0)
        endgame(obj1->colour);
      return;
    case PLANE:
      state=obj1->state;
      if (state==CRASHED || state==GHOSTCRASHED)
        return;
      if (endstatuses[obj1->index]==WINNER)
        return;
      if (type2==STARBURST || (type2==BIRD && obj1->athome))
        return;
      if (obj2==NULL) {
        if (state==FALLING) {
          stopsound(obj1);
          createexplosion(obj1);
          crater(obj1);
        }
        else
          if (state<FINISHED) {
            scoreplane(obj1);
            createexplosion(obj1);
            crater(obj1);
          }
        crashplane(obj1);
        return;
      }
      if (state>=FINISHED)
        return;
      if (state==FALLING) {
#if VERSION==7
        if (obj1->index==playerindex)
          if (type2==SHOT)
            shothole++;
          else
            if (type2==BIRD || type2==FLOCK)
              splatbird++;
#endif
        return;
      }
#if VERSION==7
      if (type2==SHOT || type2==BIRD || type2==OX || type2==FLOCK) {
        if (obj1->index==playerindex)
          if (type2==SHOT)
            shothole++;
          else
            if (type2==OX)
              splatox=TRUE;
            else
              splatbird++;
        if (state==FLYING) {
          obj1->state=WOUNDED;
          return;
        }
        if (state==STALLED) {
          obj1->state=WOUNDSTALL;
          return;
        }
      }
      else {
#else
      if (type2!=SHOT) {
#endif
        createexplosion(obj1);
        if (type2==PLANE) {
          collxadj=-collxadj;
          collxv[collptr]=((obj1->xv+obj2->xv)>>1)+collxadj;
          collyadj=-collyadj;
          collyv[collptr]=((obj1->yv+obj2->yv)>>1)+collyadj;
          collobjs[collptr++]=obj1;
        }
      }
      hitplane(obj1);
      scoreplane(obj1);
      return;
    case BIRD:
      obj1->life=(scorepenalty(type2,obj2,25) ? -1 : -2);
      return;
    case FLOCK:
      if (type2!=FLOCK && type2!=BIRD && obj1->state==FLYING) {
        for (i=0;i<8;i++)
          createbird(obj1,i);
        obj1->life=-1;
        obj1->state=FINISHED;
      }
      return;
    case OX:
      if (obj1->state!=STANDING)
        return;
      if (type2==EXPLOSION || type2==STARBURST)
        return;
      scorepenalty(type2,obj2,200);
      obj1->state=FINISHED;
      return;
  }
}

void crater(object* obj)
{
  static int depth[8]={1,2,2,3,3,2,2,1};

  int left,right,x,y,miny,maxy,i;
  left=obj->x+(obj->width-8)/2;
  right=left+7;
  for (x=left,i=0;x<=right;x++,i++)
    ground[x]=max(ground[x]-depth[i],max(originalground[x]-20,20));
  forceredraw=TRUE;
}

bool scorepenalty(int type,object* obj,int score)
{
  if (type==SHOT || type==BOMB || type==MISSILE ||
      (type==PLANE && (obj->state==FLYING || obj->state==WOUNDED ||
                       (obj->state==FALLING && obj->hitcounter==FALLCOUNT)) &&
       !obj->athome)) {
    scoretarget(obj,score);
    return TRUE;
  }
  return FALSE;
}

void scoretarget(object* destroyed,int score)
{
  if (!truemultiplayer) {
    if (destroyed->colour==MOV_CYAN)
      objects[0].score-=score;
    else
      objects[0].score+=score;
    displayscore(&objects[0]);
  }
  else {
    objects[destroyed->colour==MOV_CYAN ? 1 : 0].score+=score;
    displayscore(&objects[destroyed->colour==MOV_CYAN ? 1 : 0]);
  }
}

void scoreplane(object* destroyed)
{
  scoretarget(destroyed,50);
}

void displayscore(object* p)
{
  poscurs(p->colour==MOV_CYAN ? 2 : 9,24);
  setcolour(p->colour);
  writenum(p->score,6);
}

void timerint(void)
{
  timertick++;
}

void processtimerticks(void)
{
  for (;processedtimerticks<timertick;processedtimerticks++) {
    speedtick+=10;
    soundadjust();
  }
}

void displayplayerplane(object* plane)
{
  if (shothole!=0)
    dispwindshot();
  if (splatbird!=0)
    dispsplatbird();
  soundplane(plane);
}

void displaybomb(object* bomb)
{
  if (bomb->yv<=0)
    setsound(SOUND_BOMB,-bomb->y,bomb);
}

void displaymissile(object* missile)
{
}

void displaystarburst(object* starburst)
{
}

void displayfrag(object* frag)
{
  if (frag->subtype!=FRAG_PERSON)
    setsound(SOUND_EXPLOSION,frag->hitcounter,frag);
}

void displaycomputerplane(object* plane)
{
  soundplane(plane);
}

void displayremoteplane(object* plane)
{
  soundplane(plane);
}

void displaytarget(object* target)
{
  if (target->firing)
    setsound(SOUND_FIRING,0,target);
}

void displayflock(object* flock)
{
}

void displaybird(object* bird)
{
}

void soundplane(object* plane)
{
  if (plane->firing)
    setsound(SOUND_FIRING,0,plane);
  else
    switch (plane->state) {
      case FALLING:
        if (plane->yv>=0)
          setsound(SOUND_HIT,0,plane);
        else
          setsound(SOUND_FALLING,plane->y,plane);
        break;
      case FLYING:
        setsound(SOUND_PLANE,-plane->speed,plane);
        break;
      case STALLED:
      case WOUNDED:
      case WOUNDSTALL:
        setsound(SOUND_HIT,0,plane);
    }
}

void drawmapobject(object* obj)
{
  mapobject* mapobj=&mapobjects[obj->index];
  int oldc;
  if (mapobj->colour!=0)
    putpixel(mapobj->x,mapobj->y,mapobj->colour-1);
  if (obj->state>=FINISHED)
    mapobj->colour=0;
  else {
    mapobj->x=SCR_CENTER+(obj->x+obj->width/2)/MAPDIVX;
    mapobj->y=(obj->y-obj->height/2)/MAPDIVY;
    oldc=pixel(mapobj->x,mapobj->y,obj->owner->colour);
    if (oldc==0 || oldc==3) {
      mapobj->colour=oldc+1;
      return;
    }
    putpixel(mapobj->x,mapobj->y,oldc);
    mapobj->colour=0;
  }
}

void finish(char* msg)
{
  char* errmsg=NULL;
  setgmode(BIOS_TEXT);
  setsound(SOUND_OFF,0,NULL);
  updatesound();
  restoreints();
  flushhistory();
  printstr("\r\n");
  if (errmsg!=NULL) {
    printstr(errmsg);
    printstr("\r\n");
  }
  if (msg!=NULL) {
    printstr(msg);
    printstr("\r\n");
  }
  flushkeybuf();
  if (msg!=NULL || errmsg!=NULL)
    exit(1);
  exit(0);
}

int getmaxplayers(void)
{
  int n;
  clearwin();
  printstr(" Key maximum number of players allowed");
  while (TRUE) {
    processtimerticks();
    if (testbreak())
      finish(NULL);
    n=inkeys()-'0';
    if (n>=1 && n<=4)
      return n;
  }
}

bool updateremoteplane(object* plane)
{
  planeisplayer=planeiscomputer=FALSE;
  planeindex=plane->index;
  endstatus=endstatuses[planeindex];
  if (latencycount==0)
    processkeys(plane,getremotekeys(plane));
  else {
    plane->flaps=0;
    plane->bombing=FALSE;
  }
  if ((plane->state==CRASHED || plane->state==GHOSTCRASHED) &&
      plane->hitcounter<=0 && plane->life>QUIT) {
    plane->lives--;
    createplane(plane);
  }
  return updateplane(plane);
}

/* This function loads a section of a file to a far address */
void ffread(FILE* fp,char huge* d,unsigned long l)
{
  char buf[1024];
  unsigned long a;
  for (a=0;a<(l&-1024);a+=1024) {
    fread(buf,1024,1,fp);
    movedata(_DS,(unsigned int)buf,FP_SEG(d),FP_OFF(d),1024);
    d=(char huge*)d+1024;
  }
  if ((l&1023)!=0) {
    fread(buf,(unsigned int)(l&1023),1,fp);
    movedata(_DS,(unsigned int)buf,FP_SEG(d),FP_OFF(d),(int)l&1023);
  }
} /* ffread */

int histmult(int p,int keys)
{
  /* Dummy */
  return keys;
}

int inithistory(int randv)
{
  long historybufsize;
  FILE *inputfile;
  if (playbackfilename!=NULL || recordfilename!=NULL) {
    historybufsize=farcoreleft();
    historybuf=(short huge*)farmalloc(historybufsize);
    if (historybuf==0)
      freeandexit("Unable to allocate history memory");
    historyp=(short huge*)historybuf;
  }
  if (playbackfilename!=NULL) {
    inputfile=fopen(playbackfilename,"rb");
    if (inputfile==NULL)
      freeandexit("Unable to open history input file");
    fseek(inputfile,0,2);
    historyfilelen=ftell(inputfile);
    if (historybufsize<historyfilelen)
      freeandexit("Insufficient memory to load history file");
    historyspace=historybufsize-historyfilelen;
    fseek(inputfile,0,0);
    ffread(inputfile,(char huge*)historybuf,(int)historyfilelen);
    fclose(inputfile);
    randv=historyp[0];
    historyp+=2;
    historyfilelen-=4;
  }
  if (recordfilename!=NULL) {
    outputfile=fopen(recordfilename,"wb");
    if (outputfile==NULL)
      freeandexit("Unable to open history output file");
    if (playbackfilename==NULL) {
      historyfilelen=historybufsize;
      historyp[0]=randv;
      historyp[1]=(int)(historybufsize>>4);
      historyp+=2;
      historyfilelen-=4;
    }
  }
  return randv;
}

void freehistorybuf(void)
{
  if (historybuf!=NULL)
    farfree((void far*)historybuf);
}

void freeandexit(char* errmsg)
{
  printstr(errmsg);
  freehistorybuf();
  exit(0);
}

int history(int keys)
{
  if (playbackfilename!=NULL)
    if (keys!=0) {
      playbackfilename=NULL;
      historyfilelen+=historyspace;
    }
    else {
      if (historyp[0]==framecounter) {
        keys=historykeys=historyp[1];
        historyp+=2;
        historyfilelen-=4;
        if (historyfilelen<4) {
          playbackfilename=NULL;
          historyfilelen+=historyspace;
        }
      }
      return historykeys;
    }
  if (recordfilename!=NULL && keys!=historykeys) {
    historyp[0]=framecounter;
    historyp[1]=keys;
    recording=TRUE;
    historykeys=keys;
    historyp+=2;
    historyfilelen-=4;
    if (historyfilelen<4)
      recordfilename=NULL;
    return historykeys;
  }
  return keys;
}

/* This function saves a far block to a file */
void ffwrite(FILE* fp,char huge* d,unsigned long l)
{
  char buf[1024];
  unsigned long a;
  for (a=0;a<(l|1023)-1023;a+=1024) {
    movedata(FP_SEG((unsigned long)d+a),FP_OFF((unsigned long)d+a),_DS,
             (unsigned int)buf,1024);
    fwrite(buf,1024,1,fp);
  }
  if ((l&1023)!=0) {
    movedata(FP_SEG((unsigned long)d+a),FP_OFF((unsigned long)d+a),_DS,
             (unsigned int)buf,(int)l&1023);
    fwrite(buf,(unsigned int)(l&1023),1,fp);
  }
} /* ffwrite */

void flushhistory(void)
{
  if (recording)
    ffwrite(outputfile,(char huge*)historybuf,
            (int)(historyp-historybuf)*sizeof(int));
  if (outputfile!=NULL) {
    fclose(outputfile);
    freehistorybuf();
  }
}

int getremotekeys(object* plane)
{
  int index=plane->index;
  int state;
  int keys;
  if (gamemode==MULTIPLE) {
    if (index!=playerindex) {
      state=netbuf.states[index];
      if (plane->state!=state && (state==FINISHED || state==WAITING ||
                                  plane->state==FINISHED ||
                                  plane->state==WAITING)) {
        plane->state=state;
        usescreenbuf();
        drawmapobject(plane);
      }
    }
    keys=netbuf.keys[index];
  }
  else {
    if (index==playerindex) {
      keys=lastkey;
      /* return 0; */
    }
    else
      /* get "keys" from port */;
  }
  return histmult(index,keys);
}

void setvel(object* obj,int v,int dir)
{
  int xv=fcos(v,dir),yv=fsin(v,dir);
  obj->xv=xv>>8;
  obj->xvfrac=xv<<8;
  obj->yv=yv>>8;
  obj->yvfrac=yv<<8;
}

void printstr(char* str)
{
  while (*str!=0)
    writechar(*(str++));
}

void moveobject(object* obj,int* x,int* y)
{
  long pos,vel;
  pos=(((long)(obj->x))<<16)+obj->xfrac;
  vel=(((long)(obj->xv))<<16)+obj->xvfrac;
  pos+=vel;
  obj->x=(short)(pos>>16);
  obj->xfrac=(short)pos;
  *x=obj->x;
  pos=(((long)(obj->y))<<16)+obj->yfrac;
  vel=(((long)(obj->yv))<<16)+obj->yvfrac;
  pos+=vel;
  obj->y=(short)(pos>>16);
  obj->yfrac=(short)pos;
  *y=obj->y;
}

void writenum(int n,int size)
{
  int c=0,exponent,dig;
  bool zerof=TRUE;
  if (n<0) {
    n=-n;
    writechar('-');
    c++;
  }
  for (exponent=10000;exponent>1;exponent/=10) {
    dig=n/exponent;
    if (dig!=0 || !zerof) {
      zerof=FALSE;
      writechar(dig+'0');
      c++;
    }
    n=n%exponent;
  }
  writechar(n+'0');
  c++;
  while (++c<=size)
    writechar(' ');
}

void initasynch(void)
{
}

void createasynchremoteplane(void)
{
  object* plane;
  if (playerindex==0)
    createplayerplane(NULL);
  plane=createplane(NULL);
  plane->display_func=soundplane;
  plane->update_func=updateremoteplane;
  plane->colour=1+(plane->index&1);
  plane->owner=plane;
  plane->state=FLYING;
  memcpy(&originalplanes[plane->index],plane,sizeof(object));
  if (playerindex!=0)
    createplayerplane(NULL);
}

void getflags(int* argc,char** argv[],char* format,...)
{
  va_list ap;
  char** varg=*argv;
  int carg=*argc;
  char* arg;
  char* p;
  char* q;
  char* a;
  int i;
  int* temp;
  int** valp;
  varg++;

  while ((--carg)>0) {
    arg=*varg;
    if (*(arg++)!='-')
      break;
    if (*arg=='-') {
      varg++;
      carg--;
      break;
    }
    p=arg;
    va_start(ap,format);
    q=format;
    while (*q!=0) {
      if (*q==':')
        break;
      i=0;
      while (isalpha(q[i]))
        i++;
      if (strncmp(p,q,i)==0)
        break;
      q+=i;
      if ((*q)!=0)
        q++;
      va_arg(ap,int);
    }
    if (*p!=*q) {
      printf("Usage: ");
      a=strchr(format,':');
      if (a!=0) {
        *(a++)=0;
        q=strchr(a,'*');
        if (q!=0) {
          *(q++)=0;
          printf("%s [-%s] %s\n",a,format,q);
        }
        else
          printf("%s\n",a);
      }
      else
        printf("cmd [-%s] args\n",format);
      exit(0);
    }
    p=q+i;
    arg+=i;
    temp=(int*)va_arg(ap,int);
    valp=&temp;
    switch(*p) {
      case '#':
        a="%d";
        if (*arg=='0') {
          switch (*(++arg)) {
            case 'o':
              a="%o";
              arg++;
              break;
            case 'x':
              a="%x";
              arg++;
          }
        }
        sscanf(arg,a,*(char**)valp);
        break;
      case '*':
        **(char***)valp=arg;
        break;
      case '&':
        **(bool**)valp=TRUE;
        break;
      default:
        printf("Unknown format %c.\n",*p);
    }
    varg++;
  }
  *argv=varg;
  *argc=carg;
}

object* allocobj(void)
{
  object* newobj;
  if (nextfreeobject==NULL)
    return NULL;
  newobj=nextfreeobject;
  nextfreeobject=newobj->next;
  newobj->next=NULL;
  newobj->prev=lastobject;
  if (lastobject!=NULL)
    lastobject->next=newobj;
  else
    firstobject=newobj;
  newobj->deleteflag=newobj->drawflag=FALSE;
  newobj->tonetab=NULL;
  lastobject=newobj;
  return newobj;
}

void deleteobject(object* obj)
{
  object* other=obj->prev;
  if (other!=NULL)
    other->next=obj->next;
  else
    firstobject=obj->next;
  other=obj->next;
  if (other!=NULL)
    other->prev=obj->prev;
  else
    lastobject=obj->prev;
  obj->next=NULL;
  if (lastdeleted!=NULL)
    lastdeleted->next=obj;
  else
    firstdeleted=obj;
  lastdeleted=obj;
}

unsigned long int seed=74917777UL;

void randsd(void)
{
  seed=seed*framecounter+7491;
  if (seed==0)
    seed=74917777L;
}

void dispwindshot(void)
{
  object hole;
  hole.type=DUMMYTYPE;
  hole.height=hole.width=16;
  hole.sprite=shotsprite;
  do {
    randsd();
    drawobject((unsigned)(seed%(SCR_WIDTH-16)),
               (unsigned)(seed%(SCR_HEIGHT-50))+50,&hole);
    shothole--;
  } while (shothole!=0);
}

void dispsplatbird(void)
{
  object bird;
  bird.type=DUMMYTYPE;
  bird.height=bird.width=32;
  bird.sprite=splatsprite;
  do {
    randsd();
    drawobject((unsigned)(seed%(SCR_WIDTH-32)),
               (unsigned)(seed%(SCR_HEIGHT-60))+60,&bird);
    splatbird--;
  } while (splatbird!=0);
}

void drawobject(int x,int y,object* obj)
{
  putimage(x,y,obj->sprite,obj);
}

void putimage(int x,int y,unsigned char* p,object* obj)
{
  int linediff; unsigned int bltreg;
  unsigned char far* scp;
  int width,height,border;
  unsigned char shift,bltleft;
  shift=8-((x&3)<<1);
  width=(obj->width+3)>>2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  height=(obj->height>y+1 ? y+1 : obj->height);
  scp=scrp+((199-y)>>1)*80+(x>>2);
  linediff=interlacediff;
  if ((y&1)==0) {
    scp+=linediff;
    linediff=80-linediff;
  }
  do {
    bltleft=0;
    for (x=0;x<width;x++) {
      bltreg=((*p++)<<shift)|(bltleft<<8);
      *(scp++)^=(bltreg>>8);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft!=0)
        (*scp)^=bltleft;
    scp+=linediff-width;
    linediff=80-linediff;
    height--;
  } while (height!=0);
}

void putpixel(int x,int y,int c)
{
  int shift=(3-(x&3))<<1;
  unsigned char far* scp=scrp
                        +((199-y)>>1)*80+(x>>2)+((y&1)==0 ? interlacediff : 0);
  if ((c&0x80)==0)
    *scp=((*scp)&(~(3<<shift)))|((c&3)<<shift);
  else
    *scp^=((c&0x7f)<<shift);
}

int pixel(int x,int y,int c)
{
  int shift=(3-(x&3))<<1;
  unsigned char far* scp=scrp
                        +((199-y)>>1)*80+(x>>2)+((y&1)==0 ? interlacediff : 0);
  unsigned char o=(3<<shift)&(*scp);
  if ((c&0x80)==0) {
    *scp^=o;
    *scp|=(c<<shift);
  }
  else
    *scp^=(c&0x7f)<<shift;
  return o>>shift;
}

