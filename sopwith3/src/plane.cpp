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
#ifndef SOPWITH_PLANE_CPP
#define SOPWITH_PLANE_CPP

#include "plane.h"
#include "sopwith.h"
#include "soundsys.h"
#include "ground.h"
#include "target.h"
#include "computer.h"
#include "player.h"
#include "bullet.h"
#include "bomb.h"
#include "smoke.h"
#include "coll.h"
#include "graphics.h"
#include "bird.h"
#include "ox.h"
#include "flock.h"
#include <algorithm>

Plane::Plane(int runwayx,int colour,bool runwayleft,bool infinitefuel,bool infiniteammo,bool infinitebombs) :
  endcounter(1),
  runwayx(runwayx),
  runwayleft(runwayleft),
  infinitefuel(infinitefuel),
  infiniteammo(infiniteammo),
  infinitebombs(infinitebombs)
{
  init();
  fill();
  score=0;
  endstatus=NOTFINISHED;
  Plane::colour=colour;
  /**/
  runwayy=y.integer;
  /**/
}

void Plane::init()
{
  x=runwayx;
  y=0;
  int right=runwayx+RUNWAY_LENGTH;
  for (int i=runwayx;i<=right;i++)
    y=std::max(ground[i],y.integer);
  y.integer+=13;
  bomb_delay=0;
  hitcounter=0;
  accel=0;
  flaps=0;
  speed=0;
  setvel(0,0);
  inverted=runwayleft;
  angle=(inverted ? ANGLES/2 : 0);
  firing=false;
  bombing=false;
  goinghome=false;
  width=PLANE_WIDTH;
  height=PLANE_HEIGHT;
  athome=true;
  if (state==CRASHED)
    fill();
  state=FLYING;
  collided=false;
}

void Plane::reinit()
{
  init();
}

bool Plane::update()
{
  /**/
  if (collided) {
    xv.integer=collxv;
    yv.integer=collyv;
    collided=false;
  }
  /**/
  #ifdef DEBUG
  errorfile<<'['<<xv.integer<<','<<yv.integer<<']';
  #endif
  static char gravity[16]={0,-1,-2,-3,-4,-3,-2,-1,0,1,2,3,4,3,2,1};

  switch (state) {
    case CRASHED:
      --hitcounter;
      break;
    case FALLING:
      hitcounter-=2;
      if (yv.integer<0 && xv.integer!=0) {
        if (((inverted ? 1 : 0)^(xv.integer<0 ? 1 : 0))!=0)
          hitcounter-=flaps;
        else
          hitcounter+=flaps;
      }
      if (hitcounter<=0) {
        if (yv.integer<0) {
          if (xv.integer<0)
            ++xv;
          else {
            if (xv.integer>0)
              --xv;
            else
              inverted=!inverted;
          }
        }
        if (yv.integer>TERMINAL_VELOCITY)
          --yv;
        hitcounter=FALLCOUNT;
      }
      angle=direction()<<1;
      if (yv.integer<=0)
        initsound(SOUND_PRIORITY_PLANE_FALLING);
      break;
    case STALLED:
    case WOUNDSTALL:
    case FLYING:
    case WOUNDED:
      bool stalled;
      if (Plane::stalled()) {
        stalled=(angle!=3*ANGLES/4 || speed<minspeed);
        if (!stalled)
          state=(state==STALLED) ? FLYING : WOUNDED;
      }
      else {
        stalled=(y.integer>=MAX_Y);
        if (stalled) {
          if (gamemode==NOVICE) {
            angle=(3*ANGLES/4);
            stalled=false;
          }
          else
            stall();
        }
      }
      if (endstatus==WINNER || (gamemode!=MULTIPLE && version==2 && player->endstatus==WINNER))
        break;
      if (life<=0 && !athome && active()) {
        hit();
        scoretarget(this,SCORE_PLANE);
        return Plane::update();
      }
      int newangle=angle;
      int newspeed=speed;
      bool changedvelocity=false;
      if (flaps!=0) {
        if (inverted)
          newangle-=flaps;
        else
          newangle+=flaps;
        newangle&=(ANGLES-1);
        changedvelocity=true;
      }
      if ((framecounter&3)==0) {
        if (!stalled && newspeed<minspeed && gamemode!=NOVICE) {
          --newspeed;
          changedvelocity=true;
        }
        else {
          int limitspeed=minspeed+accel+gravity[newangle];
          if (newspeed<limitspeed) {
            ++newspeed;
            changedvelocity=true;
          }
          else
            if (newspeed>limitspeed) {
              --newspeed;
              changedvelocity=true;
            }
        }
      }
      if (changedvelocity) {
        if (athome)
          if (accel!=0 || flaps!=0)
            newspeed=minspeed;
          else
            newspeed=0;
        else
          if (newspeed<=0 && !stalled) {
            if (gamemode==NOVICE)
              newspeed=1;
            else {
              stall();
              return Plane::update();
            }
          }
        speed=newspeed;
        angle=newangle;
        if (stalled) {
          xv=0;
          yv=-newspeed;
        }
        else
          setvel(newspeed,newangle);
      }
      if (stalled) {
        if (--hitcounter==0) {
          inverted=!inverted;
          angle=(ANGLES/2-angle)&(ANGLES-1);
          hitcounter=STALLCOUNT;
        }
      }
      if (firing && ammo>0) {
        int bspeed=speed+level+BULLETSPEED;
        add(childlist,new Bullet(this,s_cos(bspeed,angle),s_sin(bspeed,angle)));
        if (!infiniteammo)
          --ammo;
      }
      if (bombing && bomb_delay==0 && bombs>0) {
        add(childlist,new Bomb(this));
        if (!infinitebombs)
          --bombs;
        bomb_delay=BOMB_DELAY;
      }
      if (!infinitefuel)
        burnfuel();
      if (speed!=0)
        athome=false;
      break;
  }
  #ifdef DEBUG
  int test;
  #endif
  if (endstatus==WINNER) {
    sprite=spritedata(PLANE_FINALE,colour==colour_cyan ? 0 : 1,endcounter/ENDTIME);
    coll=finalespritescoll[endcounter/ENDTIME];
    #ifdef DEBUG
    test=0;
    #endif
  }
  else
    if (state==FALLING && xv.integer==0 && yv.integer<0) {
      sprite=spritedata(PLANE_FALLING,colour==colour_cyan ? 0 : 1,inverted ? 1 : 0);
      coll=fallingspritescoll[inverted ? 1 : 0];
      #ifdef DEBUG
      test=2;
      #endif
    }
    else {
      sprite=spritedata(PLANE,colour==colour_cyan ? 0 : 1,inverted ? 1 : 0,angle,framecounter&1);
      coll=planespritescoll[inverted ? 1 : 0][angle];
      #ifdef DEBUG
      test=3;
      #endif
    }
  #ifdef DEBUG
  errorfile<<'('<<x.integer<<';'<<x.frac<<','<<y.integer<<';'<<y.frac<<')';
  #endif
  move();
  #ifdef DEBUG
  errorfile<<'('<<x.integer<<';'<<x.frac<<','<<y.integer<<';'<<y.frac<<')';
  #endif
  x.integer=std::max(0,std::min(x.integer,MAX_X-width));
  if (bomb_delay!=0)
    --bomb_delay;
  if (athome && state==FLYING)
    refuel();
  if (state==FALLING || wounded())
    add(childlist,new Smoke(this));
  return true;
}

void Plane::processkeys(int keys)
{
  flaps=0;
  bombing=false;
  firing=false;
  if (state==CRASHED)
    return;
  if (state!=FALLING) {
    if (endstatus!=NOTFINISHED) {
      return;
    }
    if ((keys&KEY_BREAK)!=0) {
      life=QUIT;
      goinghome=false;
      if (athome) {
        state=CRASHED;
        hitcounter=0;
      }
    }
    if ((keys&KEY_GOHOME)!=0 && flying())
      goinghome=true;
  }
  if ((framecounter&1)==1 || !wounded()) {
    if ((keys&KEY_CLIMB)!=0) {
      ++flaps;
      goinghome=false;
    }
    if ((keys&KEY_DESCEND)!=0) {
      --flaps;
      goinghome=false;
    }
    if ((keys&KEY_FLIP)!=0) {
      inverted=!inverted;
      goinghome=false;
    }
    if ((keys&KEY_BRAKE)!=0) {
      if (accel)
        --accel;
      goinghome=false;
    }
    if ((keys&KEY_ACCEL)!=0) {
      if (accel<MAX_THROTTLE)
        ++accel;
      goinghome=false;
    }
  }
  if ((keys&KEY_FIRE)!=0)
    firing=true;
  if ((keys&KEY_BOMB)!=0)
    bombing=true;
  if (goinghome)
    gohome();
}

bool Plane::onmap()
{
  return true;
}

void Plane::docollision(const Object* obj)
{
  if (state==CRASHED)
    return;
  if (endstatus==WINNER)
    return;
  if (instanceof<Smoke>(*obj) || (instanceof<Bird>(*obj) && athome))
    return;
  if (obj==0) {
    if (state==FALLING) {
      stopsound();
      createexplosion(minspeed>>(version==2||(randv&7)!=7 ? 1 : 0),6,false);
      crater();
    }
    else {
      scoretarget(this,SCORE_PLANE);
      createexplosion(minspeed>>(version==2||(randv&7)!=7 ? 1 : 0),6,flying());
      crater();
    }
    crash();
    return;
  }
  if (state==FALLING) {
    return;
  }
  if (version!=2 && (instanceof<Bullet>(*obj) || instanceof<Bird>(*obj) || instanceof<Ox>(*obj) || instanceof<Flock>(*obj))) {
    if (state==FLYING) {
      state=WOUNDED;
      return;
    }
    if (state==STALLED) {
      state=WOUNDSTALL;
      return;
    }
  }
  else
    if (/**/version!=2 ||/**/!instanceof<Bullet>(*obj)) {
      createexplosion(minspeed>>(version==2||(randv&7)!=7 ? 1 : 0),6,flying());
      if (instanceof<Plane>(*obj)) {
        collxadj=-collxadj;
        collxv=((xv.integer+obj->xv.integer)>>1)+collxadj;
        collyadj=-collyadj;
        collyv=((yv.integer+obj->yv.integer)>>1)+collyadj;
        collided=true;
        #ifdef DEBUG
        errorfile<<'#'<<collxv<<','<<collyv<<'#';
        #endif
      }
    }
  hit();
  scoretarget(this,SCORE_PLANE);
}

void Plane::sound()
{
  if (firing)
    setsound(SOUND_PRIORITY_FIRING,0,this);
  else
    switch (state) {
      case FALLING:
        if (yv.integer>=0)
          setsound(SOUND_PRIORITY_PLANE_TROUBLED,0,this);
        else
          setsound(SOUND_PRIORITY_PLANE_FALLING,y.integer,this);
        break;
      case FLYING:
        setsound(SOUND_PRIORITY_PLANE_OK,-speed,this);
        break;
      case STALLED:
      case WOUNDED:
      case WOUNDSTALL:
        setsound(SOUND_PRIORITY_PLANE_TROUBLED,0,this);
      default:
        break;
    }
}

void Plane::fill()
{
  ammo=MAX_AMMO;
  bombs=MAX_BOMBS;
  life=MAX_FUEL;
}

void Plane::gohome()
{
  if (athome)
    return;
  if (std::abs(x.integer-runwayx)<HOMEDIST && std::abs(y.integer-runwayy)<HOMEDIST) {
    reinit();
    return;
  }
  aim(runwayx,runwayy,0,false,(framecounter&0x1f)<16 ? 16 : 0);
}

void Plane::aim(int destx,int desty,Object* target,bool longway,int courseadjust)
{
  static int autoflaps[3]={0,-1,1};
  static int autorange[3];
  static bool autodoomed[3];
  static int autoalt[3];

  if (stalled() && angle!=3*ANGLES/4) {
    flaps=-1;
    if (version!=2)
      accel=MAX_THROTTLE;
    return;
  }
  int distx=x.integer-destx;
  if (std::abs(distx)>(version==2 ? 200 : 160)) {
    if (xv.integer!=0 && (distx<0)==(xv.integer<0)) {
      if (hitcounter==0)
        hitcounter=(y.integer>CRUISEHEIGHT ? 2 : 1);
      Plane::aim(x.integer,hitcounter==1 ? y.integer+25 : y.integer-25,0,true,courseadjust);
      return;
    }
    hitcounter=0;
    Plane::aim((distx<0 ? 150 : -150)+x.integer,std::min(CRUISEHEIGHT-courseadjust,y.integer+100),0,true,courseadjust);
    return;
  }
  if (!longway)
    hitcounter=0;
  if (speed!=0) {
    int disty=y.integer-desty;
    #ifdef DEBUG
    errorfile<<'^'<<y.integer<<','<<desty<<','<<disty<<'^';
    #endif
    if (disty!=0 && std::abs(disty)<6) {
      if (disty<0)
        ++y;
      else
        --y;
    }
    else {
      if (distx!=0 && std::abs(distx)<6) {
        if (distx<0)
          ++x;
        else
          --x;
      }
    }
  }
  Plane tempplane(*this);
  int newspeed=std::max(minspeed,std::min(tempplane.speed+1,maxspeed));
  for (int i=0;i<3;++i) {
    int newangle=(tempplane.angle+(tempplane.inverted ? -autoflaps[i] :
                                                         autoflaps[i]))&(ANGLES-1);
    tempplane.setvel(newspeed,newangle);
    tempplane.move();
    int newx=tempplane.x.integer,newy=tempplane.y.integer;
    autorange[i]=range(newx,newy,destx,desty);
    autoalt[i]=newy-originalground[newx+8];
    autodoomed[i]=doomed(newx,newy,autoalt[i]);
    tempplane=*this;
  }
  if (target!=0) {
    int i=gunontarget(target,&tempplane);
    if (i!=BULLET_MISS /*&& (version==2 || i==BULLET_SHORTRANGE)*/)
      firing=true;
  }
  int minrange=32767;
  int n=0; /* Initialized to suppress warning */
  for (int i=0;i<3;++i) {
    int r=autorange[i];
    if (r>=0 && r<minrange && !autodoomed[i]) {
      minrange=r;
      n=i;
    }
  }
  if (minrange==32767) {
    minrange=-32767;
    for (int i=0;i<3;++i) {
      int r=autorange[i];
      if (r<0 && r>minrange && !autodoomed[i]) {
        minrange=r;
        n=i;
      }
    }
  }
  if (speed<minspeed)
    accel=MAX_THROTTLE;
  if (minrange!=-32767) {
    if (accel<MAX_THROTTLE)
      ++accel;
  }
  else {
    if (accel!=0)
      --accel;
    n=0;
    int disty=autoalt[0];
    if (autoalt[1]>disty) {
      disty=autoalt[1];
      n=1;
    }
    if (autoalt[2]>disty)
      n=2;
  }
  flaps=autoflaps[n];
  if (flaps==0 && speed!=0)
    inverted=(xv.integer<0);
}

bool Plane::doomed(int x,int y,int alt)
{
  if (alt>50)
    return false;
  if (alt<(version==2 ? 20 : 22))
    return true;
  const std::list<Object*>::iterator end=objectlist.end();
  for (std::list<Object*>::iterator obj=objectlist.begin();obj!=end;++obj) {
    Target* target;
    Ox* ox=0;
    if (((target=dynamic_cast<Target*>(*obj))!=0 || (version!=2 && (ox=dynamic_cast<Ox*>(*obj))!=0)) &&
         (*obj)->x.integer>=x-(32+maxspeed) && (*obj)->x.integer<x+(32+maxspeed)) {
      if ((*obj)->x.integer>=x-32) {
        if ((*obj)->x.integer>x+32)
          return false;
        if (y<=(*obj)->y.integer+(((target!=0 && target->state==Target::INTACT) ||
                           (ox!=0 && ox->state==Ox::ALIVE)) ? 16 : 8))
          return true;
      }
    }
  }
  return false;
}

void Plane::stall()
{
  xv=0;
  yv=0;
  inverted=false;
  angle=7*ANGLES/8;
  speed=0;
  hitcounter=STALLCOUNT;
  state=(state==WOUNDED) ? WOUNDSTALL : STALLED;
  athome=false;
}

void Plane::hit()
{
  xv.frac=0;
  yv.frac=0;
  hitcounter=FALLCOUNT;
  state=FALLING;
  athome=false;
}

void Plane::refuel()
{
  topup(life,MAX_FUEL);
  topup(ammo,MAX_AMMO);
  topup(bombs,MAX_BOMBS);
}

bool Plane::topup(int& val,int max)
{
  bool changed=false;
  if (val!=max) {
    if (max<20) {
      if (framecounter%20==0) {
        ++val;
        changed=true;
      }
    }
    else {
      val+=max/100;
      changed=true;
    }
    val=std::min(val,max);
  }
  return changed;
}

void Plane::crash()
{
  if (xv.integer<0)
    angle=(angle+2)&(ANGLES-1);
  else
    angle=(angle-2)&(ANGLES-1);
  state=CRASHED;
  athome=false;
  xv=yv=speed=0;
  if (version==2)
    hitcounter=MAXCRCOUNT;
  else
    hitcounter=((std::abs(runwayx-x.integer)<SAFERESET) &&
                (std::abs(runwayy-y.integer)<SAFERESET)) ?
                 (MAXCRCOUNT<<1) : MAXCRCOUNT;
}

void Plane::burnfuel()
{
  life-=speed;
}

int Plane::gunontarget(Object* target,Object* tempplane)
{
  S_fix targetx=target->x;
  S_fix targety=target->y;
  S_fix targetxv=target->xv;
  S_fix targetyv=target->yv;
  S_fix bulletxv=s_cos(tempplane->speed+BULLETSPEED,tempplane->angle);
  S_fix bulletyv=s_sin(tempplane->speed+BULLETSPEED,tempplane->angle);
  S_fix bulletx=tempplane->x+PLANE_WIDTH/2;
  S_fix bullety=tempplane->y-PLANE_HEIGHT/2;
  int newangle=target->angle;
  int newspeed=target->speed;
  for (int i=0;i<BULLETLIFE;++i) {
    bulletx+=bulletxv;
    bullety+=bulletyv;
    if (version!=2) {
      Plane* targetplane=dynamic_cast<Plane*>(target);
      if (targetplane!=0 && targetplane->flying() && targetplane->flaps!=0) {
        if (targetplane->inverted)
          newangle-=targetplane->flaps;
        else
          newangle+=targetplane->flaps;
        newangle=newangle&(ANGLES-1);
        targetxv=s_cos(newspeed,newangle);
        targetyv=s_sin(newspeed,newangle);
      }
    }
    targetx+=targetxv;
    targety+=targetyv;
    int d2=range(bulletx.integer,bullety.integer,targetx.integer,targety.integer);
    if (d2<0 || d2>(version==2 ? (125*125) : (150*150)))
      return BULLET_MISS;
    if (bulletx.integer>=targetx.integer && bulletx.integer<=targetx.integer+PLANE_WIDTH-1 &&
        bullety.integer<=targety.integer && bullety.integer>=targety.integer-(PLANE_HEIGHT-1))
      return (i>BULLETLIFE/3 ? BULLET_LONGRANGE : BULLET_SHORTRANGE);
  }
  return BULLET_MISS;
}

#endif /* SOPWITH_PLANE_CPP */
