/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2001 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

#include <stdio.h>
#include <dos.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "def.h"
#include "sopasm.h"

typedef struct object
{
  int state;
  int x,y,xv,yv,angle;
  int subtype;
  bool inverted;
  int speed,accel,flaps;
  bool firing;
  int score,ammo;
  int hitcounter;
  int life;
  struct object* owner;
  int height,width;
  bool bombing;
  int bombs,colour;
  unsigned int xfrac,yfrac,xvfrac,yvfrac;
  struct object* next;
  struct object* prev;
  int index;
  int oldx,oldy;
  bool onscreen;
  unsigned char* oldsprite;
  struct object* nexterase;
  struct object* nextdraw;
  void (*draw_func)(struct object* p);
  void (*erase_func)(struct object* p);
  bool (*update_func)(struct object* p);
  struct object* nextx;
  struct object* prevx;
  int lives;
  unsigned char* sprite;
  int bomb_delay;
  bool goinghome;
  int xaux[3],yaux[3];
  int type;
  bool athome
  tonetable* tonetab;
  int missiles;
  struct object* missile_target;
  int missile_delay;
  struct object* target;
  int starbursts;
  int starburst_firing;
  int starburst_delay;
} object;

typedef struct
{
  int* planeposes;
  bool* planeflip;
  int* targetpositions;
  int* targettypes;
} landscape;

typedef struct
{
  object* erase;
  object* draw;
} redrawlist;

typedef struct
{
  int colour,x,y;
} mapobject;

int latency=-1;
int sine[16]={0x000, 0x062, 0x0b5, 0x0ed, 0x100, 0x0ed, 0x0b5, 0x062,
              0x000,-0x062,-0x0b5,-0x0ed,-0x100,-0x0ed,-0x0b5,-0x062};
int soundtype=0x7fff,soundparam=0x7fff;
object* soundobj=NULL;
unsigned int lastfreq=0;
object* lastobj=NULL;
void (*soundadjfunc)(void)=NULL;
char* sopwith1tune[7]={
  ">e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d1/",
  "d4./c8/b4/c4/d4/c+4/d4/b4/c4/c4/c4/c1/<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>d4/d4/d2./<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>e4/e4/e2./",
  "e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d2/c4/<g+4/>a4/",
  "d2/e2/g1/",
  ""};
int writecharcol=3;

bool finishflag=FALSE;

#include "sprites.c"
#include "ground.c"

int gamemode=0;
object* targets[20];
int numtargets[2];
int timertick=0;
int framecounter=0;

bool notitlesflag=FALSE;
bool soundflag=FALSE;
int screenleft=0;
int pixtoscroll=0;
int scrseg=0;
int scroff=0;
int interlacediff=0;

unsigned char auxbuf[0x2000];
bool groundtobedrawn;
object objects[MAX_OBJECTS];
object originalplanes[4];
object* lastobject;
object* firstobject;
object* nextfreeobject;
object* firstdeleted;
object* lastdeleted;
object objleft,objright;
object* compnear[4];
int endstatuses[4];
int endcounter,playerindex;
redrawlist ylist[SCR_HEIGHT];
mapobject mapobjects[MAX_OBJECTS];
int finalesx,finalesy,finalex,finaley;
bool finaleflying;
unsigned char finalesqrt[101];

int latencycount;
int endstatus;
int sndt2v[SOUNDSIZE];
int snddeltat2v[SOUNDSIZE];
int soundticks,numfrags,explplace,explline,expltone,explticks,exploctavemul;
bool titleflag;
int titleplace,titleline,titletone,titleticks,titleoctavemul;
char** tune;
int tuneline,tuneplace;
int tunefreq,tunedur;
int octavemultiplier;

object* collobj1[MAX_OBJECTS];
object* collobj2[MAX_OBJECTS];
int killptr;

bool dontcheckcoll,autoisplayer;
object* objlist[50];
int objcnt;
bool keyhandlerinstalled=FALSE;

void main(int argc,char* argv[]);
void printstr(char* str);
void updatescreen(void);
void updateobjects(void);
void update(object* obj);
bool updateplayerplane(object* p);
void processkeys(object* plane,char keys);
bool updatecomputerplane(object* plane);
bool updateremoteplane(object* plane);
bool updateplane(object* plane);
void alertcomputer(object* obj);
bool ishome(object* plane);
bool isatfinalepos(object* plane);
void refuel(object* plane);
void topup(int* val,int max);
bool updatebullet(object* bullet);
bool updatebomb(object* bomb);
int direction(object* obj);
bool updatetarget(object* target);
bool updatefrag(object* frag);
bool updatesmoke(object* smoke);
void crashplane(object* plane);
void hitplane(object* plane);
void stallplane(object* plane);
void insertx(object* ins,object* list);
void deletex(object* del);
void setsound(int type,int pitch,object* obj);
void updatesound(void);
void updatesoundint(void);
void soundadjslide(void);
void soundadjshot(void);
void explnote(void);
void titlenote(void);
void initsound(object* obj,int type);
void stopsound(object* o);
void tone(unsigned int t2);
void soundoff(void);
int soundrand(int f);
void playnote(void);
void clearwin(void);
void getgamemode(void);
int getgamenumber(void);
bool testbreak(void);
void initdisplay(bool drawnground);
void initscores(void);
void displayplanesgauge(object* p);
void displayfuelgauge(object* p);
void displaybombgauge(object* p);
void displayammogauge(object* p);
void displaygauge(int x,int h,int hmax,int col);
void drawinitialground(void);
void drawmapground(void);
void drawmaptargets(void);
void useauxbuf(void);
void usescreenbuf(void);
void initobjects(void);
void createcomputerplane(object* pl);
void createplayerplane(object* player);
object* createplane(object* pl);
void createbullet(object* plane);
void createbomb(object* plane);
void createtargets(void);
void createexplosion(object* obj);
void createsmoke(object* plane);
void setaux(object* obj);
void initcomms(void);
void createmultiplanes(void);
int getmaxplayers(void);
int inkey(void);
int flushkeybuf(void);
void setcolour(int c);
void checkcollisions(void);
void checkcollision(object* obj1,object* obj2);
void checkcrash(object* obj);
void docollision(object* obj1,object* obj2);
void scoretarget(object* destroyed,int score);
void scoreplane(object* destroyed);
void displayscore(object* p);
void writenum(int n,int width);
void crater(object* obj);
void moveobject(object* obj,int* x,int* y);
void moveaux(object* obj,int* x,int* y,int i);
void setvel(object* obj,int v,int dir);
void updateground(void);
int suny(int x);
unsigned char drawobject(int x,int y,object* p);
void eraseobject(int x,int y,object* p);
void drawplayerplane(object* plane,int x,int y);
void drawbullet(object* bullet,int x,int y);
void drawbomb(object* bomb,int x,int y);
void drawtarget(object* target,int x,int y);
void drawfrag(object* frag,int x,int y);
void drawsmoke(object* smoke,int x,int y);
void drawmapobject(object* obj);
void drawcomputerplane(object* plane,int x,int y);
void drawremoteplane(object* plane,int x,int y);
void planesound(object* plane);
void erasebullet(object* bullet,int x,int y);
void erasebomb(object* bomb,int x,int y);
void erasetarget(object* buliding,int x,int y);
void erasefrag(object* frag,int x,int y);
void erasesmoke(object* smoke,int x,int y);
void eraseplayerplane(object* plane,int x,int y);
void erasecomputerplane(object* plane,int x,int y);
void eraseremoteplane(object* plane,int x,int y);
void finish(char* msg,bool closef);
void endgame(int n);
void winner(object* player);
void loser(object* plane);
int pixel(int x,int y,int c);
object* allocobj(void);
void deleteobject(object* obj);
void startsound(void);
void timerint(void);
void computerpilot(object* plane);
void attack(object* plane,object* attackee);
bool gunontarget(object* plane,object* target);
int cruise(object* plane);
int flytofinale(object* plane);
int aim(object* plane,int destx,int desty,object* bpa);
bool ailogic(object* plane,int ch1,int ch2,int rdepth);
bool checkcrash(object* plane,int xleft,int ybottom,int rot);
void mkobjlist(object* plane);
object* hitobj(object* plane,int rdepth,int xleft,int ybottom);
int range(int x0,int y0,int x1,int y1);
unsigned char bitblt(int x,int y,unsigned char* p,int w,int h);
void getopt(int* argc,char** argv[],char* format,...);
char* finishcomms(bool closef);
char getremotekey(object* player);

void main(int argc,char* argv[])
{
  int i;
  getopt(&argc,&argv,"k#s&:sopwith*",
         &latency,          /* -k[num] (control latency) */
         &soundflag         /* -s (sound flag) */
        );
  inittimer(timerint);
  setgmode(BIOS_GRAPHICS);
  if (!notitlesflag) {
    setsound(SOUND_TITLE,0,NULL);
    updatesound();
    setcolour(3); poscurs(13,9); printstr("S O P W I T H");
    setcolour(1); poscurs(12,12); printstr("BMB "); setcolour(3); printstr("Compuscience");
  }
  getgamemode();
  multiplayer=(gamemode==MULTIPLE);
  if (multiplayer) {
    if (latency==-1)
      latency=2;
    initcomms();
    clearwin();
    initobjects();
    createmultiplanes();
  }
  else {
    if (latency==-1)
      latency=1;
    clearwin();
    initobjects();
    createplayerplane(NULL);
    for (i=0;i<3;i++)
      createcomputerplane(NULL);
  }
  createtargets();
  initdisplay(FALSE);
  while (TRUE) {
    updateobjects();
    checkcollisions();
    updatescreen();
    updatesound();
  }
}

void erasebullet(object* bullet)
{
}

void erasebomb(object* bomb)
{
}

void erasetarget(object* target)
{
}

void erasefrag(object* frag)
{
}

void erasesmoke(object* smoke)
{
}

void eraseplayerplane(object* plane)
{
}

void erasecomputerplane(object* plane)
{
  if (plane->oldsprite==NULL)
    plane->deleteflag=FALSE;
}

void eraseremoteplane(object* plane)
{
  if (plane->oldsprite==NULL)
    plane->deleteflag=FALSE;
}

void drawplayerplane(object* plane)
{
  if (plane->state!=FINISHED)
    planesound(plane);
  else
    plane->drawflag=FALSE;
}

void drawbullet(object* bullet)
{
}

void drawbomb(object* bomb)
{
  if (bomb->yv<=0)
    setsound(SOUND_BOMB,-bomb->y,bomb);
}

void drawtarget(object* target)
{
}

void drawfrag(object* frag)
{
  setsound(SOUND_EXPLOSION,frag->hitcounter,frag);
}

void drawsmoke(object* smoke)
{
}

void drawcomputerplane(object* plane)
{
  if (plane->state!=FINISHED)
    planesound(plane);
  else
    plane->drawflag=FALSE;
}

void drawremoteplane(object* plane)
{
  if (plane->state!=FINISHED)
    planesound(plane);
  else
    plane->drawflag=FALSE;
}

void updatescreen(void)
{
  int bytestomove,bytestoscroll,offset,ioffset,scrolltop,y,temp;
  object* obj;
  redrawlist* objl;
  usescreenbuf();
  bytestoscroll=pixtoscroll>>2;
  if (bytestoscroll<0) {
    bytestomove=bytestoscroll+80;
    offset=0;
  }
  else {
    offset=bytestoscroll;
    bytestomove=80-bytestoscroll;
  }
  scrolltop=75;
  if (ground[screenleft-pixtoscroll]>=scrolltop)
    scrolltop=ground[screenleft-pixtoscroll];
  if (ground[(screenleft+SCR_WIDTH-1)-pixtoscroll]>=scrolltop)
    scrolltop=ground[(screenleft+SCR_WIDTH-1)-pixtoscroll];
  if (finaleflying)
    if (finalesy+50>=scrolltop)
      scrolltop=finalesy+50;
  scrolltop|=1;
  offset+=((199-scrolltop)>>1)*80;
  ioffset=interlacediff+offset;
  objl=&ylist[SCR_HEIGHT-1];
  for (y=SCR_HEIGHT-1;y>=0;y--) {
    while (obj=objl->erase;obj!=NULL && !groundtobedrawn;obj=obj->nexterase) {
      obj->deleteflag=TRUE;
      if (obj->erase_func!=NULL)
        obj->erase_func(obj);
      if (obj->deleteflag && obj->oldx>=0 && obj->oldx<SCR_WIDTH &&
          obj->oldy>=0 && obj->oldy<SCR_HEIGHT)
        putimage(obj->oldx,obj->oldy,obj->oldsprite,obj->width,obj->width);
    }
    objl->erase=NULL;
    if (y>=16 && y<=scrolltop) {
      farmemmove(MK_FP(scrseg,offset),MK_FP(scrseg,offset-bytestoscroll),
                 bytestomove);
      temp=ioffset;
      ioffset=offset+80;
      offset=temp;
    }
    for (obj=objl->draw;obj!=NULL;obj=obj->nextdraw) {
      obj->onscreen=FALSE;
      if (obj->x>=screenleft && obj->x<=screenleft+SCR_WIDTH-1) {
        obj->onscreen=TRUE;
        obj->oldx=obj->x-screenleft;
        obj->oldy=obj->y;
        obj->drawflag=TRUE;
        if (obj->draw_func!=NULL)
          obj->draw_func(obj);
        if (obj->drawflag && obj->oldx>=0 && obj->oldx<SCR_WIDTH &&
            obj->oldy>=0 && obj->oldy<SCR_HEIGHT)
          putimage(obj->oldx,obj->oldy,obj->sprite,obj->width,obj->width);
      }
    }
    objl->draw=NULL;
    objl--;
  }
  groundtobedrawn=FALSE;
  updateground();
}

void updateground(void)
{
  int xl,xr,x,y,xs,x0,x1,xs0,xs1;
  if (pixtoscroll<0) {
    xl=screenleft;
    xr=xl-(pixtoscroll+1);
    xs=0;
  }
  else {
    xr=screenleft+SCR_WIDTH-1;
    xl=xr+1-pixtoscroll;
    xs=320-pixtoscroll;
  }
  for (x=xl;x<=xr;x++,xs++) {
    x1=ground[x];
    x0=ground[x-pixtoscroll];
    if (x1>x0)
      for (y=x1;y>x0;y--)
        putpixel(xs,y,XOR_WHITE);
    else
      for (y=x1+1;y<=x0;y++)
        putpixel(xs,y,XOR_WHITE);
    if (finaleflying) {
      xs0=suny(x);
      xs1=suny(x-pixtoscroll);
      for (y=x0+1;y<=xs1;y++)
        putpixel(xs,y,XOR_MAGENTA);
      for (y=x1+1;y<=xs0;y++)
        putpixel(xs,y,XOR_MAGENTA);
    }
  }
}

int suny(int x)
{
  if (x<finalesx-50 || x>finalesx+50)
    return ground[x];
  return finalesqrt[x+50-finalesx];
}

void updateobjects(void)
{
  object* next;
  object* current;
  bool oldonscreen;
  int ydraw,yerase;
  redrawlist* ydlist;
  if (firstdeleted!=NULL) {
    lastdeleted->next=nextfreeobject;
    nextfreeobject=firstdeleted;
    lastdeleted=firstdeleted=NULL;
  }
  latencycount++;
  if (latencycount>=latency)
    latencycount=0;
  current=firstobject;
  while (current!=NULL) {
    next=current->next;
    oldonscreen=current->onscreen;
    if (oldonscreen) {
      yerase=current->y;
      ydraw=yerase-current->width+1;
    }
    current->oldsprite=current->sprite;
    if (current->update_func(current)) {
      if (current->onscreen)
        if (current->y<yerase)
          ydraw=(current->y-current->width)+1;
        else
          yerase=current->y;
      else
        ydraw=(current->y-current->width)+1;
      ydlist=&ylist[ydraw];
      current->nextdraw=ydlist->draw;
      ydlist->draw=current;
    }
    else
      current->onscreen=FALSE;
    if (oldonscreen) {
      ydlist=&ylist[yerase];
      current->nexterase=ydlist->erase;
      ydlist->erase=current;
    }
    current=next;
  }
  framecounter++;
}

bool updateplayerplane(object* plane)
{
  bool result;
  int keys;
  int xv,x;
  endstatus=endstatuses[plane->index];
  if (endstatus!=NOTFINISHED && endcounter>0) {
    endcounter--;
    if (endcounter<=0)
      finish(NULL,TRUE);
  }
  if (latencycount==0) {
    if (multiplayer)
      keys=getremotekeys(plane);
    else {
      keys=inkey();
      flushkeybuf();
    }
    processkeys(plane,keys);
  }
  else {
    plane->flaps=0;
    plane->accel=0;
    plane->bombing=FALSE;
  }
  if (plane->state==CRASHED && plane->hitcounter<=0) {
    if (endstatus!=WINNER && ((--(plane->lives))<=0 || plane->life<=QUIT)) {
      if (endstatus==0) {
        loser(plane);
        if (multiplayer)
          endgame(1);
      }
    }
    else {
      createplayerplane(plane);
      initdisplay(TRUE);
      if (endstatus==WINNER) {
        winner(plane);
      }
    }
  }
  result=updateplane(plane);
  if (endstatus==2 || (endstatus==1 && isatfinalepos(plane)))
    pixtoscroll=0;
  else {
    x=plane->x-screenleft;
    xv=plane->xv;
    if (x<152) {
      if (xv<0)
        pixtoscroll=-((-xv)|3)-1;
    }
    else
      if (x>152 && xv>0)
        pixtoscroll=1+(xv|3);
    if (pixtoscroll<0) {
      if (x>=232)
        pixtoscroll=0;
    }
    else
      if (pixtoscroll>0 && x<=72)
        pixtoscroll=0;
    if (pixtoscroll+screenleft<0 || pixtoscroll+screenleft+SCR_WIDTH-1>=MAX_X)
      pixtoscroll=0;
    screenleft+=pixtoscroll;
  }
  displayfuelgauge(plane);
  if (ishome(plane)) {
    displayammogauge(plane);
    displaybombgauge(plane);
    displayplanesgauge(plane);
  }
  else {
    if (plane->firing)
      displayammogauge(plane);
    if (plane->bombing)
      displaybombgauge(plane);
  }
  return result;
}

void processkeys(object* plane,char keys)
{
  plane->flaps=0;
  plane->accel=0;
  plane->bombing=FALSE;
  plane->firing=FALSE;
  if (plane->state!=FLYING && plane->state!=STALLED)
    return;
  if (endstatus!=NOTFINISHED) {
    if (endstatus==LOSER && plane->index==playerindex)
      flytofinale(plane);
    else
      cruise(plane);
    return;
  }
  switch (keys) {
    case 1:
      plane->life=QUIT;
      plane->goinghome=FALSE;
      if (ishome(plane)) {
        plane->state=CRASHED;
        plane->hitcounter=0;
      }
      break;
    case ',':
      plane->flaps=1;
      plane->goinghome=FALSE;
      break;
    case '/':
      plane->flaps=-1;
      plane->goinghome=FALSE;
      break;
    case '.':
      plane->inverted=!(plane->inverted);
      plane->goinghome=FALSE;
      break;
    case '\\':
      plane->accel=-1;
      plane->goinghome=FALSE;
      break;
    case 'x':
      plane->accel=1;
      plane->goinghome=FALSE;
      break;
    case ' ':
      plane->firing=TRUE;
      break;
    case 'b':
      plane->bombing=TRUE;
      break;
    case 'h':
      plane->goinghome=TRUE;
      break;
    case 's':
      if (plane->index==playerindex) {
        if (soundflag) {
          setsound(SOUND_OFF,0,NULL);
          updatesound();
        }
        soundflag=!soundflag;
      }
      break;
    default:
      break;
  }
  if (plane->goinghome)
    cruise(plane);
}

bool updatecomputerplane(object* plane)
{
  plane->flaps=0;
  plane->accel=0;
  plane->bombing=FALSE;
  endstatus=endstatuses[plane->index];
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
        cruise(plane);
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

bool updateremoteplane(object* plane)
{
  endstatus=endstatuses[plane->index];
  if (latencycount==0)
    processkey(plane,getremotekey(plane));
  else {
    plane->flaps=0;
    plane->accel=0;
    plane->bombing=FALSE;
  }
  if (plane->state==CRASHED && plane->hitcounter<=0 && plane->life>QUIT &&
      (--(plane->lives))>0)
    createplane(plane);
  return updateplane(plane);
}

bool updateplane(object* plane)
{
  int newangle,maxv,x,y,newspeed;
  switch (plane->state) {
    case FINISHED:
    case WAITING:
      return FALSE;
    case CRASHED:
    case GHOSTCRASHED:
      plane->hitcounter--;
      break;
    case FALLING:
      plane->hitcounter--;
      if (plane->hitcounter==0) {
        if (plane->yv>TERMINAL_VELOCITY)
          plane->yv--;
       plane->hitcounter=5;
      }
      if (plane->yv<=0)
        initsound(plane,SOUND_FALLING);
      break;
    case FLYING:
    case STALLED:
      if (plane->life<=0 && !ishome(plane)) {
        hitplane(plane);
        scoreplane(plane);
        return updateplane(plane);
      }
      if (plane->state==STALLED && plane->angle==3*ANGLES/4)
        plane->state=FLYING;
      if (plane->state==FLYING && plane->y>=MAX_Y)
        stallplane(plane);
      if (plane->flaps!=0 || plane->accel!=0) {
        newspeed=plane->speed;
        newangle=plane->angle;
        newspeed+=plane->accel;
        if (plane->inverted)
          newangle-=plane->flaps;
        else
          newangle+=plane->flaps;
        maxv=(finaleflying ? 4 : 8);
        if (newspeed<4)
          newspeed=4;
        else
          if (newspeed>maxv)
            newspeed=maxv;
        plane->speed=newspeed;
        newangle&=(ANGLES-1);
        plane->angle=newangle;
        if (plane->state==FLYING)
          setvel(plane,newspeed,newangle);
        else {
          plane->xv=0;
          plane->yv=-newspeed;
          plane->xvfrac=0;
          plane->yvfrac=0;
        }
      }
      if (plane->state==STALLED) {
        plane->hitcounter--;
        if (plane->hitcounter==0) {
          plane->inverted=!(plane->inverted);
          plane->angle=(ANGLES/2-plane->angle)&(ANGLES-1);
          plane->hitcounter=2;
        }
      }
      if (plane->firing)
        createbullet(plane);
      if (plane->bombing)
        createbomb(plane);
      plane->life-=plane->speed;
  }
  if (endstatus==1 && plane->index==playerindex && isatfinalepos(plane))
    plane->sprite=finalesprites[(plane->colour-1)&1][endcounter/ENDTIME];
  else
    if (plane->state==FINISHED)
      plane->sprite=NULL;
    else
      if (plane->state==FALLING)
        plane->sprite=planesprites[(plane->colour-1)&1]
                                  [plane->inverted ? 1 : 0]
                                  [direction(plane)<<1];
      else
        plane->sprite=planesprites[(plane->colour-1)&1]
                                  [plane->inverted ? 1 : 0]
                                  [plane->angle];
  moveobject(plane,&x,&y);
  if (x<0)
    x=plane->x=0;
  else
    if (x>=MAX_X-16)
      x=plane->x=MAX_X-16;
  if (plane->state==FLYING || plane->state==STALLED)
    alertcomputer(plane);
  deletex(plane);
  insertx(plane,plane->nextx);
  if (plane->bomb_delay!=0)
    plane->bomb_delay--;
  if (ishome(plane))
    refuel(plane);
  if (y<MAX_Y && y>=0) {
    if (plane->state==FALLING || plane->state==WOUNDED || plane->state==WOUNDSTALL)
      createsmoke(plane);
    usescreenbuf();
    drawmapobject(plane);
    return TRUE;
  }
  return FALSE;
}

void alertcomputer(object* obj)
{
  object* plane;
  object* attackee;
  int i;
  for (i=0,plane=firstobject;i<MAX_PLAYERS;i++,plane++) {
    if (plane->update_func==updatecomputerplane &&
        obj->owner->colour!=plane->owner->colour) {
      attackee=compnear[i];
      if (attackee==NULL ||
          (attackee->type==TARGET && obj->type==PLANE)) {
        if (gamemode==COMPUTER && abs(originalplanes[i].x-obj->x)<=600)
          compnear[i]=obj;
      }
      else
        if (attackee->type!=PLANE || obj->type!=TARGET &&
            abs(obj->x-plane->x)<abs(attackee->x-plane->x))
          compnear[i]=obj;
    }
  }
}

bool ishome(object* plane)
{
  object* home=&originalplanes[plane->index];
  return (plane->x==home->x && plane->y==home->y && plane->speed==0 &&
          plane->state==FLYING);
}

bool isatfinalepos(object* plane)
{
  return (plane->x==finalex && plane->y==finaley && plane->speed==0 &&
          plane->state==FLYING);
}

void refuel(object* plane)
{
  topup(&plane->life,MAX_FUEL);
  topup(&plane->ammo,MAX_AMMO);
  topup(&plane->bombs,MAX_BOMBS);
}

bool updatetarget(object* target)
{
  if (target->state==STANDING) {
    alertcomputer(target);
    target->sprite=targetsprites[(target->owner->colour-1)&1]
                                  [target->subtype];
  }
  else
    target->sprite=debrissprites[(target->owner->colour-1)&1];
  return TRUE;
}

bool updatefrag(object* frag)
{
  int x,y;
  deletex(frag);
  frag->life--;
  if (frag->life==0) {
    stopsound(frag);
    deleteobject(frag);
    return FALSE;
  }
  moveobject(frag,&x,&y);
  if (x<0 || x>=MAX_X || y>=MAX_Y || y<=ground[x]) {
    stopsound(frag);
    deleteobject(frag);
    return FALSE;
  }
  frag->hitcounter++;
  frag->sprite=fragsprites[frag->subtype];
  insertx(frag,frag->nextx);
  return TRUE;
}

void hitplane(object* plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->angle=3*ANGLES/4;
  plane->speed=0;
  plane->hitcounter=5;
  plane->state=FALLING;
}

void stallplane(object* plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->xv=0;
  plane->yv=-6;
  plane->inverted=FALSE;
  plane->angle=7*ANGLES/8;
  plane->speed=6;
  plane->hitcounter=2;
  plane->state=(plane->state>=GHOST) ? GHOSTSTALLED :
                                       ((plane->state==WOUNDED) ? WOUNDSTALL :
                                                                  STALLED);
}

void updatesound(void)
{
  int i;
  int* deltat2vp;
  int* t2vp;
  for (i=0,t2vp=sndt2v,deltat2vp=snddeltat2v;i<SOUNDSIZE;i++,t2vp++,deltat2vp++)
    if (*t2vp!=0)
      (*t2vp)+=(*deltat2vp)*soundticks;
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
      break;
  }
  soundparam=0x7fff;
  soundtype=SOUND_NONE;
}

void soundadjslide(void)
{
  int i=lastobj->speed;
  tone(snddeltat2v[i]*soundticks+sndt2v[i]);
}

void initsound(object* obj,int type)
{
  int i;
  if (obj->speed!=0)
    return;
  if (obj->type==EXPLOSION) {
    numfrags++;
    if (numfrags==1) {
      explplace=explline=0;
      explnote();
    }
    obj->speed=1;
    return;
  }
  for (i=0;i<SOUNDSIZE;i++)
    if (sndt2v[i]==0) {
      switch(type) {
        case SOUND_BOMB:
          sndt2v[i]=0x300;
          snddeltat2v[i]=8;
          break;
        case SOUND_FALLING:
          sndt2v[i]=0x1200;
          snddeltat2v[i]=-8;
          break;
      }
      obj->speed=i;
      return;
    }
}

void stopsound(object* obj)
{
  int s=obj->speed;
  if (s==0)
    return;
  if (obj->type==EXPLOSION)
    numfrags--;
  else {
    sndt2v[s]=0;
    snddeltat2v[s]=0;
  }
  o->speed=0;
}

void initscores(void)
{
  displayscore(&objects[0]);
}

void drawinitialground(void)
{
  int gx=screenleft;
  int x,h,y;
  for (x=0;x<SCR_WIDTH;x++,gx++) {
    h=ground[gx];
    for (y=16;y<=h;y++)
      putpixel(x,y,XOR_WHITE);
  }
}

void drawmapground(void)
{
  int dx=0,maxy=0,x,sx=SCR_CENTER,y;
  for (x=0;x<MAX_X;x++) {
    if (ground[x]>maxy)
      maxy=ground[x];
    dx++;
    if (dx==MAPDIVX) {
      h/=MAPDIVY;
      for (y=0;y<=h;y++)
        putpixel(sx,y,XOR_WHITE);
      sx++;
      maxy=dx=0;
    }
  }
}

void drawmaptargets(void)
{
  object* target;
  int i;
  for (i=0;i<MAX_OBJECTS;i++)
    mapobjects[i].colour=0;
  for (i=0;i<MAX_TARGETS;i++) {
    target=targets[i];
    if (target!=NULL && target->state!=FINISHED)
      drawmapobject(target);
  }
}

void createplayerplane(object* player)
{
  object* plane=createplane(player);
  if (player==NULL) {
    plane->draw_func=drawplayerplane;
    plane->erase_func=eraseplayerplane;
    plane->update_func=updateplayerplane;
    plane->colour=((plane->index&1)==0 ? MOV_CYAN : MOV_MAGENTA);
    plane->owner=plane;
    memcpy(&originalplanes[plane->index],plane,sizeof(object));
  }
  screenleft=((plane->x-153)|3)+1;
  pixtoscroll=0;
  flushkeybuf();
}

object* createplane(object* pl)
{
  static int runwayx[8]={1270,588,1330,1360,1630,1660,2464,1720};
  static bool runwayleft[8]={FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE};
  int singlerunways[2]={0,7};
  int computerrunways[4]={0,7,1,6};
  int multiplerunways[8]={0,7,3,4,2,5,1,6};

  int x,y,right,left,runway;
  object* plane=(pl!=NULL ? pl : allocobj());
  switch(gamemode) {
    case SINGLE:
    case NOVICE:
      runway=singleplanes[plane->index];
      break;
    case MULTIPLE:
    case ASYNCH:
      runway=multipleplanes[plane->index];
      break;
    case COMPUTER:
      runway=computerplanes[plane->index];
      break;
  }
  plane->type=PLANE;
  left=plane->x=runwayx[runway];
  right=left+RUNWAY_LENGTH;
  y=0;
  for (x=left;x<=right;x++)
    if (ground[x]>y)
      y=ground[x];
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
  setaux(plane);
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
  if (pl==NULL) {
    plane->score=0;
    plane->ammo=MAX_AMMO;
    plane->bombs=MAX_BOMBS;
    plane->lives=MAXLIVES;
    plane->life=MAX_FUEL;
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

void createbullet(object* obj)
{
  int i,x,y;
  object* bullet;
  for (i=0;i<2;i++) {
    if (obj->ammo==0)
      return;
    bullet=allocobj();
    if (bullet==NULL)
      return;
    obj->ammo--;
    bullet->type=SHOT;
    setvel(bullet,obj->speed+BULLETSPEED,obj->angle);
    bullet->x=obj->x+PLANE_WIDTH/2;
    bullet->y=obj->y-PLANE_HEIGHT/2;
    bullet->xfrac=obj->xfrac;
    bullet->yfrac=obj->yfrac;
    if (i!=0)
      moveobject(bullet,&x,&y);
    setaux(bullet);
    bullet->life=10;
    bullet->owner=obj;
    bullet->colour=obj->colour;
    bullet->width=1;
    bullet->draw_func=drawbullet;
    bullet->erase_func=erasebullet;
    bullet->update_func=updatebullet;
    bullet->speed=0;
    insertx(bullet,obj);
  }
}

void createbomb(object* plane)
{
  object* bomb; /* Someone set us up the bomb! Sorry. */
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
  bomb->x=plane->x+(fcos(8,rot)>>8)+4;
  bomb->y=plane->y+(fsin(8,rot)>>8)-4;
  bomb->xfrac=bomb->yfrac=bomb->xvfrac=bomb->yvfrac=0;
  bomb->life=BOMBLIFE;
  bomb->owner=plane;
  bomb->colour=plane->colour;
  bomb->width=bomb->height=8;
  bomb->draw_func=drawbomb;
  bomb->erase_func=erasebomb;
  bomb->update_func=updatebomb;
  insertx(bomb,plane);
}

void createtargets(void)
{
  static int targetx[MAX_TARGETS]={
    191,284,409,539,685,807,934,1210,1240,1440,
    1550,1750,1780,2024,2159,2279,2390,2549,2678,2763};
  static int targettype[MAX_TARGETS]={
    TARGET_CHIMNEY,TARGET_TANK,TARGET_CHIMNEY,TARGET_CHIMNEY,TARGET_TANK,
    TARGET_FLAG,TARGET_CHIMNEY,TARGET_FUEL,TARGET_FLAG,TARGET_TANK,
    TARGET_TANK,TARGET_FLAG,TARGET_FUEL,TARGET_CHIMNEY,TARGET_CHIMNEY,
    TARGET_TANK,TARGET_TANK,TARGET_FLAG,TARGET_FLAG,TARGET_CHIMNEY};

  object* target;
  int i,left,right,minaltitude,maxaltitude,gx,y;
  numtargets[0]=0;
  numtargets[1]=MAX_TARGETS-3;
  for (i=0;i<MAX_TARGETS;i++) {
    targets[i]=target=allocobj();
    target->x=minx=targetx[i];
    maxx=minx+15;
    minaltitude=999;
    maxaltitude=0;
    for (x=minx;x<=maxx;x++) {
      if (ground[x]>maxaltitude)
        maxaltitude=ground[x];
      if (ground[x]<minaltitude)
        minaltitude=ground[x];
    }
    y=(minaltitude+maxaltitude)>>1;
    if (y>MAX_Y-17)
      y=MAX_Y-17;
    target->y=y+16;
    for (x=minx;x<=maxx;x++)
      ground[gx]=y;
    target->xv=0;
    target->yv=0;
    target->xfrac=0;
    target->yfrac=0;
    target->xvfrac=0;
    target->yvfrac=0;
    target->angle=0;
    setaux(target);
    target->type=TARGET;
    target->state=STANDING;
    target->subtype=targettype[i];
    target->life=i;
    target->owner=&objects[i>=MAX_TARGETS/2-3 && i<MAX_TARGETS/2 ? 1 : 0];
    target->colour=target->owner->colour;
    target->height=target->width=16;
    target->draw_func=drawtarget;
    target->erase_func=erasetarget;
    target->update_func=updatetarget;
    insertx(target,&objleft);
  }
}

void createexplosion(object* obj)
{
  int l,step,i;
  long randno;
  object* frag;
  if (obj->update_func==updatetarget && obj->subtype==TARGET_FUEL) {
    step=1;
    l=10;
  }
  else {
    step=2;
    l=5;
  }
  for (i=1;i<=15;i+=step) {
    frag=allocobj();
    if (frag==NULL)
      return;
    frag->type=EXPLOSION;
    setvel(frag,8,i);
    frag->x=frag->xv+obj->x+obj->width/2;
    frag->y=frag->yv+obj->y-obj->width/2;
    frag->xfrac=0;
    frag->yfrac=0;
    setaux(frag);
    randno=(long)frag->x*(long)frag->y*0xbe8b71f5l;
    frag->life=(int)(((((unsigned long)((unsigned short)(randno*((unsigned long)i))))*((unsigned long)l))>>16)+1);
    frag->subtype=(int)((((unsigned long)((unsigned short)(randno*((unsigned long)i))))*8UL)>>16);
    frag->hitcounter=0;
    frag->owner=obj;
    frag->colour=obj->colour;
    frag->width=8;
    frag->draw_func=drawfrag;
    frag->erase_func=erasefrag;
    frag->update_func=updatefrag;
    frag->speed=0;
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
  smoke->width=smoke->height=1;
  smoke->draw_func=drawsmoke;
  smoke->erase_func=erasesmoke;
  smoke->update_func=updatesmoke;
  smoke->colour=plane->colour;
}

void setaux(object* obj)
{
  int i;
  for (i=0;i<3;i++) {
    obj->xaux[i]=obj->x;
    obj->yaux[i]=obj->y;
  }
}

void createmultiplanes(void)
{
}

void checkcollisions(void)
{
  int right,top,bottom,i,type,left;
  object** obj1;
  object** obj2;
  object* test;
  object* obj;
  int w;
  killptr=0;
  useauxbuf();
  for (obj=objleft.nextx;obj!=&objright;obj=obj->nextx) {
    right=left+obj->width-1;
    bottom=obj->y;
    top=bottom-(obj->height-1);
    for (test=obj->nextx;test->x<=right && test!=&objright;test=test->nextx) {
      left=test->x;
      if (test->y>top && test->y-test->width+1<=bottom)
        checkcollision(obj,test);
    }
    type=obj->type;
    if (obj!=NULL)
      if ((type==PLANE && obj->state!=FINISHED && obj->state!=WAITING &&
           bottom<ground[left+8]+24) ||
          ((type==BOMB || type==MISSILE) && bottom<ground[left+4]+12))
        checkcrash(obj);
  }
  for (i=0,obj1=collobj1,obj2=collobj2;i<killptr;i++,obj1++,obj2++)
    docollision(*obj1,*obj2);
}

void checkcollision(object* obj1,object* obj2)
{
  int type1=(obj1!=NULL ? obj1->type : GROUND);
  int type2=(obj2!=NULL ? obj2->type : GROUND);
  object* t;
  if ((type1==PLANE && obj1->state>=FINISHED) ||
      (type2==PLANE && obj2->state>=FINISHED) ||
      (type1==EXPLOSION && type2==EXPLOSION))
    return;
  if (obj1->y<obj2->y) {
    t=obj1;
    obj1=obj2;
    obj2=t;
  }
  drawobject(15,15,obj1);
  if (drawobject(15+obj2->x-obj1->x,15+obj2->y-obj1->y,obj2) && killptr<149) {
    collobj1[killptr]=obj1;
    collobj2[killptr++]=obj2;
    collobj1[killptr]=obj2;
    collobj2[killptr++]=obj1;
  }
  clearcolltestarea();
}

void docollision(object* obj1,object* obj2)
{
  int type2=(obj2!=NULL ? obj2->type : GROUND);
  if (obj1==NULL)
    return;
  switch (obj1->type) {
    case BOMB:
    case MISSILE:
      createexplosion(obj1);
      obj1->life=-1;
      if (type2==GROUND)
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
      obj1->life=1;
      stopsound(obj1);
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
      if (obj1->state==CRASHED)
        return;
      if (type2==GROUND) {
        createexplosion(obj1);
        if (obj1->state!=FALLING) {
          scoreplane(obj1);
          stopsound(obj1);
        }
        crashplane(obj1);
        return;
      }
      if (obj1->state==FALLING) {
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
      if (type2!=SHOT)
        createexplosion(obj1);
      hitplane(obj1);
      scoreplane(obj1);
      return;
  }
}

void scoretarget(object* destroyed,int score)
{
  if (destroyed->colour==1)
    objects[0].score-=score;
  else
    objects[0].score+=score;
  displayscore(&objects[0]);
}

void displayscore(object* p)
{
  poscurs(p->colour==MOV_CYAN ? 1 : 8,24);
  setcolour(p->colour);
  writenum(p->score,6);
}

void crater(object* obj)
{
  static int depth[8]={1,2,2,3,3,2,2,1};

  int left,right,scleft,scright,sx,x,osleft,osright,osx,ny,y,oy,cx;
  usescreenbuf();
  left=obj->x+(obj->width-8)/2;
  right=obj->width+left-1;
  scleft=screenleft-pixtoscroll;
  scright=(screenleft+SCR_WIDTH-1)-pixtoscroll;
  sx=left-scleft;
  osleft=((originalplanes[playerindex].x-153)|3)+1;
  osright=osleft+319;
  osx=left-osleft;
  for (x=left,cx=0;x<=right;x++,cx++,sx++,osx++) {
    oy=ground[x];
    ny=(oy-depth[cx])+1;
    if (ny<=loground[x])
      ny=loground[x]+1;
    ground[x]=ny-1;
    if (x>=scleft && x<=scright)
      for (y=oy;y>=ny;y--)
        putpixel(sx,y,XOR_WHITE);
    if (x>=osleft && x<=osright) {
      useauxbuf();
      for (y=oy;y>=ny;y--)
        putpixel(osx,y,XOR_WHITE);
      usescreenbuf();
    }
  }
  useauxbuf();
}

void moveobject(object* obj,int* x,int* y)
{
  long pos,vel;
  int i;
  pos=(((long)(obj->x))<<16)+obj->xfrac;
  vel=(((long)(obj->xv))<<16)+obj->xvfrac;
  pos+=vel;
  obj->x=(short)(pos>>16);
  obj->xfrac=(short)pos;
  *x=obj->x;
#if VERSION==1
  for (i=0;i<3;i++) {
    pos+=(vel<<1);
    obj->xaux[i]=(short)(pos>>16);
  }
#endif
  pos=(((long)(obj->y))<<16)+obj->yfrac;
  vel=(((long)(obj->yv))<<16)+obj->yvfrac;
  pos+=vel;
  obj->y=(short)(pos>>16);
  obj->yfrac=(short)pos;
  *y=obj->y;
#if VERSION==1
  for (i=0;i<3;i++) {
    pos+=(vel<<1);
    obj->yaux[i]=(short)(pos>>16);
  }
#endif
}

/* Let me tell you a story. I had fixed all the bugs I found after decompiling,
except for one - the autopilot wasn't working - it decided that the best way to
get anywhere was to go straight up. I decompiled all the autopilot routines
again, checking everything I could possibly think of to check. Everything
seemed fine except for one minor detail - it didn't work. Since I didn't know
how the autopilot worked, I had no idea what execution path it should be
taking. So finally I decided the only way to find the bug was to compare it to
the original. By hand, I crafted and assembled a piece of code to automatically
send the plane home after a short journey, and patched into the executable with
a hex editor. I ran the C debugger in one DOS session and D86 in another, and
stepped through the programs in synchronization. Finally I found that this
routine was returning 0 in *x and *y when it shouldn't have been - I had
forgotten to convert to long before shifting left by 16. That bug sets a
personal record for how long it took me to find - 3 days. */

void moveaux(object* obj,int* x,int* y,int i)
{
  long pos,vel;
  pos=(((long)(obj->xaux[i]))<<16)|0x8000L;
  vel=(((long)(obj->xv))<<16)|((long)(obj->xvfrac));
  (*x)=obj->xaux[i]=(short)((pos+vel)>>16);
  pos=(((long)(obj->yaux[i]))<<16)|0x8000L;
  vel=(((long)(obj->yv))<<16)|((long)(obj->yvfrac));
  (*y)=obj->yaux[i]=(short)((pos+vel)>>16);
}

unsigned char drawobject(int x,int y,object* p)
{
  if (x>=0 && x<SCR_WIDTH && y>=0 && y<SCR_HEIGHT)
    return bitblt(x,y,p->sprite,p->width,p->width);
  return 0;
}

void eraseobject(int x,int y,object* p)
{
}

void drawmapobject(object* obj)
{
  mapobject* mapobj=&mapobjects[obj->index];
  int c;
  if (mapobj->colour!=0)
    putpixel(mapobj->x,mapobj->y,mapobj->colour-1);
  if (obj->state==FINISHED)
    mapobj->colour=0;
  else {
    c=putpixel(mapobj->x=(obj->x+obj->width/2)/19+152,
               mapobj->y=(obj->y-obj->width/2)/13,obj->owner->colour);
    if (c==0 || c==3) {
      mapobj->colour=c+1;
      return;
    }
    putpixel(mapobj->x,mapobj->y,c);
    mapobj->colour=0;
  }
}

void planesound(object* plane)
{
  if (plane->firing)
    setsound(SOUND_FIRING,0,plane);
  else
    switch(plane->state) {
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
        setsound(SOUND_HIT,0,plane);
    }
}

void endgame(int n)
{
  object* player;
  for (player=firstobject;player->type==PLANE;player=player->next)
    if (endstatuses[player->index]==0)
      if (player->colour==1)
        winner(player);
      else
        loser(player);
}

void winner(object* player)
{
  int index,x,gy,x0,y,yy;
  endstatuses[index=player->index]=1;
  if (index==playerindex) {
    if (player->x>MAX_X/2)
      finalesx=screenleft-60;
    else
      finalesx=screenleft+SCR_WIDTH+60-1;
    finalesy=ground[finalesx-50];
    for (x=finalesx-49;finalesx+50>=x;x++) {
      gy=ground[x];
      if (gy<finalesy)
        finalesy=gy;
    }
    for (x=0;x<=100;x++) {
      x0=x-50;
      yy=50*50-x0*x0;
      for (y=0;y*y<yy;y++); /* 1 line sqrt */
      finalesqrt[x]=finalesy+y;
    }
    endcounter=200;
    finalex=finalesx-8;
    finaley=ground[finalex]+30;
    finaleflying=TRUE;
  }
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
  newobj->onscreen=FALSE;
  lastobject=newobj;
  return newobj;
}

void computerpilot(object* plane)
{
  object* target=compnear[plane->index];
  autoisplayer=FALSE;
  if (target!=NULL)
    attack(plane,target);
  else
    cruise(plane);
  compnear[plane->index]=NULL;
}

void attack(object* plane,object* attackee)
{
  if (plane->life<=abs(plane->x-originalplanes[plane->index].x)+500 ||
      plane->ammo<=0) {
    cruise(plane);
    return;
  }
  if (attackee->type==PLANE && attackee->speed!=0)
    aim(plane,attackee->x-(fcos(32,attackee->angle)>>8),
              attackee->y-(fsin(32,attackee->angle)>>8),attackee);
  else
    aim(plane,attackee->x,attackee->y,attackee);
}

bool gunontarget(object* plane,object* target)
{
  object tempbullet,temptarget;

  int bulletx,bullety,targetx,targety;
  int d2,i;
  memcpy(&tempbullet,plane,sizeof(object));
  memcpy(&temptarget,target,sizeof(object));
  setvel(&tempbullet,tempbullet.speed+BULLETSPEED,tempbullet.angle);
  tempbullet.x+=PLANE_WIDTH/2;
  tempbullet.y-=PLANE_HEIGHT/2;
  for (i=0;i<BULLETLIFE;i++) {
    moveobject(&tempbullet,&bulletx,&bullety);
    moveobject(&temptarget,&targetx,&targety);
    d2=range(bulletx,bullety,targetx,targety);
    if (d2<0 || d2>NEAR)
      return FALSE;
    if (bulletx>=targetx && bulletx<=targetx+PLANE_WIDTH-1 &&
        bullety<=targety && bullety>=targety-(PLANE_HEIGHT-1))
      return TRUE;
  }
  return FALSE;
}

int cruise(object* plane)
{
  object* home;
  if (ishome(plane))
    return 0;
  home=&originalplanes[plane->index];
  if (abs(plane->x-home->x)<16 && abs(plane->y-home->y)<16) {
    if (plane->update_func==updateplayerplane) {
      createplayerplane(plane);
      initdisplay(TRUE);
    }
    else
      if (plane->update_func==updatecomputerplane)
        createcomputerplane(plane);
      else
        createplane(plane);
    return 0;
  }
  autoisplayer=TRUE;
  return aim(plane,home->x,home->y,NULL);
}

int flytofinale(object* plane)
{
  if (isatfinalepos(plane))
    return 0;
  if (abs(plane->x-finalex)<16 && abs(plane->y-finaley)<16) {
    createplane(plane);
    plane->x=finalex;
    plane->y=finaley;
    endcounter=72;
    setcolour(XOR_MAGNETA);
    poscurs(16,12);
    printstr("THE END");
    return 0;
  }
  autoisplayer=TRUE;
  return aim(plane,finalex,finaley,NULL);
}

typedef struct
{
  int flaps,accel,xv,yv,xvfrac,yvfrac,newspd,alt,d2;
} autoinf;

autoinf choices[5]={{ 0, 0,0,0,0,0,0,0,0},
                    {-1, 0,0,0,0,0,0,0,0},
                    { 1, 0,0,0,0,0,0,0,0},
                    { 0,-1,0,0,0,0,0,0,0},
                    { 0, 1,0,0,0,0,0,0,0}};

int aim(object* plane,int destx,int desty,object* destobj)
{
  autoinf *choice;
  object tplane;
  object* ptplane;
  int px,py,distx,disty,i,ch,ch2,ch3;
  int newrot,newspd,maxspd,d2,mindist2;
  bool fire=FALSE;
  dontcheckcoll=FALSE;
  maxspd=finaleflying ? 4 : 8;
  if (plane->state==STALLED && plane->angle!=3*ANGLES/4) {
    plane->flaps=-1;
    return 0x8001;
  }
  if (ishome(plane) && (plane->life<6000 || plane->ammo<133 || plane->bombs<3))
    return 0x8001;
  px=plane->x;
  py=plane->y;
  distx=px-destx;
  if (abs(distx)>200) {
    py+=100;
    return aim(plane,px+(distx>=0 ? -150 : 150),
                     (py<=180 ? py : 180),destobj);
  }
  if (plane->speed!=0) {
    disty=py-desty;
    if (disty!=0 && abs(disty)<6)
      plane->y=((disty<0) ? ++py : --py);
    else
      if (distx && abs(distx)<6)
        plane->x=((distx<0) ? ++px : --px);
  }
  memcpy(ptplane=&tplane,plane,sizeof(object));
  for (i=0,choice=choices;i<5;i++,choice++) {
    newrot=(ptplane->angle+(ptplane->inverted ? -choice->flaps :
                                                   choice->flaps))&15;
    newspd=ptplane->speed+choice->accel;
    if (newspd>maxspd)
      newspd=maxspd;
    else
      if (newspd<4)
        newspd=4;
    setvel(ptplane,newspd,newrot);
    distx=choice->xv=ptplane->xv;
    disty=choice->yv=ptplane->yv;
    choice->xvfrac=ptplane->xvfrac;
    choice->yvfrac=ptplane->yvfrac;
    choice->newspd=newspd;
    choice->alt=disty+py-loground[distx+px+8];
    choice->d2=range(px+distx,py+disty,destx,desty);
  }
  setaux(ptplane);
  mkobjlist(plane);
  d2=choices[0].d2;
  if (destobj!=NULL && d2>=0 && d2<=75*75 && gunontarget(ptplane,destobj)) {
    ch=0;
    fire=TRUE;
  }
  else {
    mindist2=32767;
    for (i=0,choice=choices;i<5;i++,choice++) {
      d2=choice->d2;
      if (d2>=0 && d2<mindist2) {
        mindist2=d2;
        ch=i;
      }
    }
    if (mindist2==32767) {
      mindist2=-32767;
      for (i=0,choice=choices;i<5;i++,choice++) {
        d2=choice->d2;
        if (d2<0 && d2>mindist2) {
          mindist2=d2;
          ch=i;
        }
      }
    }
  }
  if (ailogic(ptplane,ch,ch,1)) {
    ch2=ch;
    fire=FALSE;
    dontcheckcoll=TRUE;
    disty=-32767;
    for (i=0,choice=choices;i<5;i++,choice++)
      if (i!=ch2 && choice->alt>disty) {
        disty=choice->alt;
        ch=i;
      }
    if (ailogic(ptplane,ch,ch,1)) {
      ch3=ch;
      disty=-32767;
      for (i=0,choice=choices;i<5;i++,choice++)
        if (i!=ch2 && i!=ch3 && (choice->xv!=0 || choice->yv>0) &&
            choice->alt>disty) {
          disty=choice->alt;
          ch=i;
        }
      if (disty==-32767 || ailogic(ptplane,ch,ch,1)) {
        dontcheckcoll=FALSE;
        disty=32767;
        if (py>107) {
          for (i=0,choice=choices;i<5;i++,choice++)
            if (i!=ch2 && choice->alt<disty) {
              disty=choice->alt;
              ch=i;
            }
          if (ailogic(ptplane,ch,ch,1))
            ch=ch2;
        }
        else
          ch=ch2;
      }
    }
  }
  plane->flaps=choices[ch].flaps;
  plane->accel=choices[ch].accel;
  if (fire)
    plane->firing=TRUE;
  if (plane->flaps==0 && plane->accel==0)
    if (ishome(plane))
      plane->accel=-1;
    else
      if (plane->speed!=0)
        plane->inverted=(plane->xv<0);
  return choices[ch].d2;
}

bool ailogic(object* plane,int ch1,int ch2,int rdepth)
{
  autoinf *choice2=&choices[ch2];
  autoinf *choice1=&choices[ch1];
  int ch,newspd,newrot,xa1,ya1,xa0,ya0;
  bool f;

  xa0=plane->xaux[0];
  ya0=plane->yaux[0];
  newspd=choice1->newspd;
  newrot=(plane->angle+(plane->inverted ? -choice2->flaps :
                                              choice2->flaps)*rdepth)&15;
  setvel(plane,newspd,newrot);
  moveaux(plane,&xa1,&ya1,0);
  moveaux(plane,&xa1,&ya1,0);
  if ((!dontcheckcoll && ground[xa1+8]+24>ya1 &&
       (!autoisplayer || checkcrash(plane,xa1,ya1,newrot))) || ya1>=200)
    f=TRUE;
  else
    if (hitobj(plane,rdepth,xa1,ya1)!=NULL)
      f=TRUE;
    else
      if (rdepth>=3)
        f=FALSE;
      else
        if (!ailogic(plane,ch1,ch2,++rdepth))
          f=FALSE;
        else {
          f=TRUE;
          for (ch=0;ch<3;ch++)
            if (ch!=ch2 && !ailogic(plane,ch1,ch,rdepth)) {
              f=FALSE;
              break;
            }
        }
  plane->xaux[0]=xa0;
  plane->yaux[0]=ya0;
  return f;
}

bool checkcrash(object* plane,int xleft,int ybottom,int angle)
{
  int y,right,x;
  bool hit=FALSE;
  useauxbuf();
  putimage(15,15,planesprites[0][plane->inverted ? 1 : 0][angle],16,16);
  right=xleft+plane->width-1;
  for (x=xleft;x<=right;x++) {
    y=((int)ground[x]-ybottom)+15;
    if (y>15) {
      hit=TRUE;
      break;
    }
    if (y>=0) {
      hit=pixel((x-xleft)+15,y,XOR_BLACK);
      if (hit)
        break;
    }
  }
  clearcolltestarea();
  return hit;
}

void mkobjlist(object* plane)
{
  object* obj;
  int sleft,sright,left,right,bottom,top,oleft,oright,obottom,otop,type;
  sleft=plane->x-160;
  sright=sleft+320;
  left=plane->x-48;
  right=plane->x+63;
  top=plane->y+48;
  bottom=plane->y-63;
  objcnt=-1;
  for (obj=plane->prevx;obj!=&objleft;obj=obj->prevx)
    if (obj->x<sleft)
      break;
  for (obj=obj->nextx;obj!=&objright;obj=obj->nextx) {
    if (obj->x>sright || objcnt>=49)
      break;
    if (obj!=plane) {
      type=(obj!=NULL) ? obj->type : 0;
      if ((type!=PLANE || obj->state!=FINISHED) &&
          (type!=SHOT || obj->owner!=plane)) {
        if (obj->xv>0) {
          oleft=obj->x;
          oright=obj->xaux[2]+obj->width-1;
        }
        else {
          oleft=obj->xaux[2];
          oright=obj->x+obj->width-1;
        }
        if (obj->yv>0) {
          obottom=(obj->y-obj->width)+1;
          otop=obj->yaux[2];
        }
        else {
          obottom=(obj->yaux[2]-obj->width)+1;
          otop=obj->y;
        }
        if (oleft<=right && otop>=bottom && oright>=left && obottom<=top)
          objlist[++objcnt]=obj;
      }
    }
  }
}

object* hitobj(object* plane,int rdepth,int xleft,int ybottom)
{
  object* obj;
  int left,right,top,bottom,x,y,objno;
  left=xleft;
  right=xleft+15;
  top=ybottom-15;
  bottom=ybottom;
  for (objno=0;objno<=objcnt;objno++) {
    obj=objlist[objno];
    x=obj->xaux[rdepth-1];
    y=obj->yaux[rdepth-1];
    if (x<=right && y>=top && x+obj->width-1>=left && y+1-obj->width<=bottom)
      return obj;
  }
  return NULL;
}

int range(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
  if (x<100 && y<100)
    return x*x+y*y;
  if (x<y) { t=x; x=y; y=t; }
  return -((x*7+(y<<2))>>3);
}


unsigned char blut[256]={
0x00,0x03,0x03,0x03,0x0c,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff
};

unsigned char bitblt(int x,int y,unsigned char* p,int height,int width)
{
  unsigned int linediff,outpos,startpos,yleft,bltreg;
  unsigned char retval=0,shift,bltleft;
  unsigned char far *scp;
  int border;
  if (height==1 && width==1)
    return pixel(x,y,(int)p);
  scp=(unsigned char far *)MK_FP(scrseg,scroff);
  shift=8-((x&3)<<1);
  y=199-y;
  width>>=2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  yleft=(y+height>200 ? 200-y : height);
  linediff=interlacediff;
  startpos=((y>>1)*320+x)>>2;
  if (y&1) {
    startpos+=linediff;
    linediff=80-linediff;
  }
  for (y=0;y<yleft;y++) {
    outpos=startpos;
    bltleft=0;
    for (x=0;x<width;x++) {
      bltreg=((*(p++))<<shift)|(bltleft<<8);
      retval|=scp[outpos]&blut[bltreg>>8];
      scp[outpos++]^=(bltreg>>8);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft) {
        retval|=scp[outpos]&blut[bltleft];
        scp[outpos]^=bltleft;
      }
    startpos+=linediff;
    linediff=80-linediff;
  }
  return retval;
}

void initcomms(void)
{
}

char* finishcomms(bool closef)
{
  return NULL;
}

char getremotekey(object* player)
{
  int i,state=WAITING,key=0;
  if (finishflag)
    finish(NULL,TRUE);
  i=player->index;
  if (i!=playerindex) {
    timertick=0;
    if (player->state!=state &&
        (state==FINISHED || state==WAITING ||
         player->state==FINISHED || player->state==WAITING)) {
      player->state=state;
      usescreenbuf();
      drawmapobject(player);
    }
  }
  return key;
}

