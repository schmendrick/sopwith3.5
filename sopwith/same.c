/* Source code for Sopwith
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2001 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

void printstr(char* str)
{
  while (*str!=0)
    writechar(*(str++));
}

void getopt(int* argc,char** argv[],char* format,...)
{
  va_list ap;
  char** varg=*argv;
  int carg=*argc;
  char* arg;
  char* p;
  char* q;
  char* a;
  int i;
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
    do {
      if (*arg==0 && *p!='&') {
        varg+=2;
        arg=*varg;
        carg--;
      }
      valp=(int**)(&va_arg(ap,int));
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
          sscanf(arg,a,*valp);
          break;
        case '*':
          **valp=(int)arg;
          break;
        case '&':
          **valp=(int)TRUE;
          break;
        default:
          printf("Unknown format %c.\n",*p);
      }
      if (*p=='&')
        if (*arg==0)
          break;
    } while (TRUE);
    varg+=2;
  }
  *argv=varg;
  *argc=carg;
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

void clearwin(void)
{
  int i;
  for (i=20;i<23;i++) {
    poscurs(0,i);
    clearline();
  }
  poscurs(0,20);
}

void getgamemode(void)
{
  clearwin();
  printstr("Key: S - single player\r\n");
  printstr("     M - multiple players\r\n");
  printstr("     C - single player against computer");
  while (TRUE) {
    if (testbreak())
      finish(NULL,FALSE);
    switch (inkey()) {
      case 's':
        gamemode=SINGLE;
        return;
      case 'm':
        gamemode=MULTIPLE;
        return;
      case 'c':
        gamemode=COMPUTER;
        return;
    }
  }
}

int getgamenumber(void)
{
  int n;
  clearwin();
  printstr("         Key a game number");
  while (TRUE) {
    if (testbreak())
      finish(NULL,FALSE);
    n=inkey()-'0';
    if (n>=0 && n<=7)
      return n;
  }
}

bool testbreak(void)
{
  return 0;
}

int soundrand(int f)
{
  static int seed[50]={
    0x90b9,0xbcfb,0x6564,0x3313,0x3190,0xa980,0xbcf0,0x6f97,0x37f4,0x064b,
    0x9fd8,0x595b,0x1eee,0x820c,0x4201,0x651e,0x848e,0x15d5,0x1de7,0x1585,
    0xa850,0x213b,0x3953,0x1eb0,0x97a7,0x35dd,0xaf2f,0x1629,0xbe9b,0x243f,
    0x847d,0x3a31,0x3295,0xbc11,0x6e6d,0x3398,0xad43,0x51ce,0x8f95,0x507e,
    0x499e,0x3bc1,0x5243,0x2017,0x9510,0x9865,0x65f6,0x6b56,0x36b9,0x5026};
  static int i=0;
  if (i>=50)
    i=0;
  return seed[i++]%f;
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

void soundadjshot(void)
{
  static unsigned int savefreq;
  if (lastfreq==0xf000)
    tone(savefreq);
  else {
    savefreq=lastfreq;
    tone(0xf000);
  }
}

void updatesoundint(void)
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

void explnote(void)
{
  tuneline=explline;
  tuneplace=explplace;
  tune=sopwith1tune;
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
  tune=sopwith1tune;
  octavemultiplier=titleoctavemul;
  playnote();
  titleline=tuneline;
  titleplace=tuneplace;
  titletone=tunefreq;
  titleticks+=tunedur;
  titleoctavemul=octavemultiplier;
  tone(titletone);
}


void timerint(void)
{
  timertick++;
  updatesoundint();
}

void loser(object* plane)
{
  int player=plane->index;
  endstatuses[player]=LOSER;
  if (player==playerindex) {
    setcolour(XOR_MAGNETA);
    poscurs(16,12);
    printstr("THE END");
    endcounter=20;
  }
}

void useauxbuf(void)
{
  scrseg=_DS;
  scroff=(int)(&auxbuf)-0x1000;
  interlacediff=0x1000;
}

void useonscreenbuf(void)
{
  scrseg=SCR_SEG;
  scroff=0;
  interlacediff=0x2000;
}

void setcolour(int c)
{
  writecharcol=c;
}

void writenum(int n,int width)
{
  int c=0,zerof=1,exponent,dig;
  if (n<0) {
    n=-n;
    writechar('-');
    c++;
  }
  for (exponent=10000;exponent>1;exponent/=10) {
    if ((dig=n/exponent)!=0 || zerof==0) {
      zerof=0;
      writechar(dig+'0');
      c++;
    }
    n=n%exponent;
  }
  writechar(n+'0');
  c++;
  do {
    c++;
    if (c>width)
      break;
    writechar(' ');
  } while (TRUE);
}


void setvel(object* obj,int v,int dir)
{
  int xv=fcos(v,dir),yv=fsin(v,dir);
  obj->xv=xv>>8;
  obj->xvfrac=xv<<8;
  obj->yv=yv>>8;
  obj->yvfrac=yv<<8;
}

void scoreplane(object* destroyed)
{
  scoretarget(destroyed,50);
}

int flushkeybuf(void)
{
  while (kbhit())
    getch();
  return 0;
}

int inkey(void)
{
  if (kbhit())
    return getch();
  return 0;
}

int getmaxplayers(void)
{
  int n;
  clearwin();
  printstr(" Key maximum number of players allowed");
  while (TRUE) {
    if (testbreak())
      finish(NULL,FALSE);
    n=inkey()-'0';
    if (n>=1 && n<=4)
      return n;
  }
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
  displaygauge(BOMBGAUGEX,player->bombs,MAX_BOMBS,MOV_CYAN+MOV_MAGENTA-player->colour);
}

void displayammogauge(object* player)
{
  displaygauge(AMMOGAUGEX,player->ammo,MAX_AMMO,MOV_WHITE);
}

void topup(int* val,int max)
{
  if (*val!=max) {
    if (max<20) {
      if (framecounter%20==0)
        (*val)++;
    }
    else
      *val+=max/100;
    if (*val>max)
      *val=max;
  }
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
  bullet->sprite=(unsigned char*)XOR_WHITE;
  return TRUE;
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
  bomb->sprite=bombsprites[bomb->owner->colour==MOV_CYAN ? 0 : 1]
                          [direction(bomb)];
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
  if (xv==0) {if (yv<0) return 6; if (yv>0) return 2; return 6; }
  if (xv>0)  {if (yv<0) return 7; if (yv>0) return 1; return 0; }
              if (yv<0) return 5; if (yv>0) return 3; return 4;
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
  smoke->sprite=(unsigned char*)(XOR_BLACK+smoke->colour);
  return TRUE;
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

void crashplane(object* plane)
{
  if (plane->xv<0)
    plane->angle=(plane->angle+2)&(ANGLES-1);
  else
    plane->angle=(plane->angle-2)&(ANGLES-1);
  plane->state=(plane->state>=GHOST ? GHOSTCRASHED : CRASHED);
  plane->xv=plane->yv=plane->xvfrac=plane->yvfrac=plane->speed=0;
  plane->hitcounter=MAXCRCOUNT;
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

void playnote(void)
{
  int majorscale[7]={0,2,3,5,7,8,10};
  int notefreq[12]={440,466,494,523,554,587,622,659,698,740,784,831};
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
      else {
        switch (tunechar) {
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
  }
  durbuf[duri]=0;
  duration=atoi(tempobuf);
  if (duration<=0)
    duration=4;
  if (noteletter=='R')
    freq=0x7d00;
  else {
    semitone+=sharpen;
    while (semitone<0) {
      semitone+=12;
      overflowoct>>=1;
    }
    while (semitone>=12) {
      semitone-=12;
      overflowoct<<=1;
    }
    freq=(short)(((long)notefreq[semitone]*(long)octavemultiplier*
                  (long)noteoctave)>>16);
  }
  tunefreq=(short)(1193181L/freq);
  tunedur=((dotdur*TEMPO)/(tempo*60))>>1;
}

void initdisplay(bool drawnground)
{
  object* player;
  if (!drawnground) {
    useauxbuf();
#if VERSION==1
    drawinitialground();
#endif
    drawmapground();
    if (!notitlesflag) {
      setsound(SOUND_OFF,0,NULL);
      updatesound();
    }
  }
  farmemset(MK_FP(SCR_SEG,0),0x1000,0);
  farmemset(MK_FP(SCR_SEG,0x2000),0x1000,0);
  farmemmove(MK_FP(_DS,(int)(&auxbuf)),MK_FP(SCR_SEG,0x1000),0x1000);
  farmemmove(MK_FP(_DS,(int)(&auxbuf)+0x1000),MK_FP(SCR_SEG,0x3000),0x1000);
  usescreenbuf();
  drawmaptargets();
  initscores();
  player=&objects[playerindex];
  displayfuelgauge(player);
  displaybombgauge(player);
  displayammogauge(player);
  displayplanesgauge(player);
  groundtobedrawn=TRUE;
}

void displaygauge(int x,int h,int hmax,int colour)
{
  int y;
  h=(h*GAUGE_HEIGHT)/hmax-1;
  if (h>GAUGE_HEIGHT-1)
    h=GAUGE_HEIGHT-1;
  for (y=0;y<=h;y++)
    putpixel(x,y,colour);
  for (;y<GAUGE_HEIGHT;y++)
    putpixel(x,y,MOV_BLACK);
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

void createcomputerplane(object* pl)
{
  object* plane=createplane(pl);
  if (pl==NULL) {
    plane->draw_func=drawcomputerplane;
    plane->erase_func=erasecomputerplane;
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

void checkcrash(object* obj)
{
  int x,right,y;
  bool hit=FALSE;
  draw(15,15,obj);
  right=obj->x+obj->width-1;
  for (x=obj->x;x<=right;x++) {
    y=(int)ground[x]+15-obj->y;
    if (y>15) {
      hit=TRUE;
      break;
    }
    if (y>=0) {
      hit=pixel(x+15-obj->x,y,XOR_BLACK);
      if (hit)
        break;
    }
  }
  clearcolltestarea();
  if (hit && killptr<(MAX_OBJECTS<<1)) {
    collobj1[killptr]=obj;
    collobj2[killptr++]=NULL;
  }
}

void finish(char* msg,bool closef)
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

int pixel(int x,int y,int c)
{
  int bit=(3-x&3)<<1;
  unsigned char far *scp=(unsigned char far *)MK_FP(scrseg,scroff+((((199-y)>>1)*320+x)>>2)+(((~y)&1)==1 ? interlacediff : 0));
  unsigned char prev=(*scp)&(3<<bit);
  if ((c&0x80)==0) {
    *scp^=prev;
    *scp|=c<<bit;
    return (prev<<bit);
  }
  *scp^=(c&0x7f)<<bit;
  return (prev<<bit);
}


