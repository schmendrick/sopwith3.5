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
#ifndef SOPWITH_LIBNET_NETWORK_H
#define SOPWITH_LIBNET_NETWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <libnet.h>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include "../resource.h"
#include "../graphics.h"
#include "../keyboard.h"
#include "../sopwith.h"
#include "../sw_excep.h"

bool networkavailable=true;
NET_CONN* server;
std::list<NET_CONN*> clients;
std::map<NET_CONN*,int> conntoposition;
std::map<int,int> positiontokeys;
std::set<int> vacantpositions;

static unsigned short twobytes=0xff;
static bool littleendian=*reinterpret_cast<unsigned char*>(&twobytes)!=0;

inline short tos(short word)
{
  if (littleendian)
    return (((static_cast<short>(word) & 0xff00)>>8) |
            ((static_cast<short>(word) & 0x00ff)<<8));
  else
    return word;
}

class Send_Receive {
public:
  virtual ~Send_Receive() {}
  virtual void add(bool& data)=0;
  virtual void add(short& data)=0;
  virtual void add(int& data)=0;
};

class Send : public Send_Receive {
public:
  Send(NET_CONN* conn)
  {
    conns_.push_back(conn);
  }
  Send(const std::list<NET_CONN*>& conns) : conns_(conns) {}
  virtual void add(bool& data)
  {
    short temp=tos(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual void add(short& data)
  {
    short temp=tos(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual void add(int& data)
  {
    short temp=tos(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual ~Send()
  {
    std::string s=out.str();
    const std::list<NET_CONN*>::const_iterator end=conns_.end();
    for (std::list<NET_CONN*>::const_iterator iter=conns_.begin();iter!=end;++iter)
      net_send_rdm(*iter,s.data(),s.length());
  }
private:
  std::list<NET_CONN*> conns_;
  std::stringstream out;
};

class Receive : public Send_Receive {
public:
  Receive(NET_CONN* conn) : conn_(conn),index(0)
  {
    int size;
    while ((size=net_query_rdm(conn))==0);
    buffer=new char[size];
    net_receive_rdm(conn,buffer,size);
  }
  virtual void add(bool& data)
  {
    short temp=*reinterpret_cast<short*>(buffer+index);
    index+=sizeof(temp);
    data=tos(temp);
  }
  virtual void add(short& data)
  {
    short temp=*reinterpret_cast<short*>(buffer+index);
    index+=sizeof(temp);
    data=tos(temp);
  }
  virtual void add(int& data)
  {
    short temp=*reinterpret_cast<short*>(buffer+index);
    index+=sizeof(temp);
    data=tos(temp);
  }
  ~Receive()
  {
    delete[] buffer;
  }
private:
  NET_CONN* conn_;
  char* buffer;
  int index;
};

class Driverlist {
public:
  inline Driverlist()
  : driverlist_(net_driverlist_create())
  {
  }
  inline Driverlist(int driver)
  : driverlist_(net_driverlist_create())
  {
    net_driverlist_add(driverlist_,driver);
  }
  inline Driverlist(const NET_DRIVERLIST& driverlist)
  : driverlist_(net_driverlist_create())
  {
    net_driverlist_add_list(driverlist_,driverlist);
  }
  inline ~Driverlist()
  {
    net_driverlist_destroy(driverlist_);
  }
  inline operator NET_DRIVERLIST()
  {
    return driverlist_;
  }
private:
  NET_DRIVERLIST driverlist_;
};

template<class T>
inline T Drivernames(const NET_DRIVERLIST& driverlist)
{
  NET_DRIVERNAME* drivernames_=net_getdrivernames(driverlist);
  T result(drivernames_,drivernames_+net_driverlist_count(driverlist));
  free(drivernames_);
  return result;
}

void initnetwork()
{
  Resource graphicsobject(textmode,graphicsmode);
  Resource keyboardobject(deinitkeyboard,initkeyboard);
  net_init();
  net_loadconfig(0);
  Driverlist candidatedrivers(net_drivers_all);
  net_driverlist_remove(candidatedrivers,NET_DRIVER_NONET);
  net_driverlist_remove(candidatedrivers,NET_DRIVER_LOCAL);

  std::vector<NET_DRIVERNAME> detecteddrivers=Drivernames<std::vector<NET_DRIVERNAME> >(net_detectdrivers(candidatedrivers));
  int choice;

  Send_Receive* send_receive;

  if (connmode==SERVER) {
    std::vector<NET_DRIVERNAME> initializeddrivers;
    while (!detecteddrivers.empty()) {
      for (std::vector<NET_DRIVERNAME>::size_type i=0;i<detecteddrivers.size();++i)
        std::cout<<i<<": "<<detecteddrivers[i].name<<" ("<<net_classes[net_driver_class(detecteddrivers[i].num)].name<<")\n";
      std::cout<<"Choose driver to be initialized (0-"<<detecteddrivers.size()-1<<") or enter something else to continue: ";
      std::string line;
      getline(std::cin,line);
      std::stringstream interpreter;
      interpreter<<line;
      if ((interpreter>>choice) && (interpreter>>std::ws).eof() &&
          std::cin && choice>=0 && choice<static_cast<int>(detecteddrivers.size())) {
        std::cout<<"\nInitializing driver...\n";
        net_initdrivers(Driverlist(detecteddrivers[choice].num));
        initializeddrivers.push_back(detecteddrivers[choice]);
        detecteddrivers.erase(detecteddrivers.begin()+choice);
      }
      else {
        if (initializeddrivers.empty())
          std::cout<<"\nAt least one driver must be initialized.\n";
        else
          break;
      }
      std::cout<<'\n';
    }
    std::vector<NET_CONN*> listens;
    listens.reserve(initializeddrivers.size());
    std::cout<<"\nListening for clients using the following drivers:\n\n";
    const std::vector<NET_DRIVERNAME>::const_iterator end1=initializeddrivers.end();
    for (std::vector<NET_DRIVERNAME>::iterator driver=initializeddrivers.begin();driver!=end1;++driver) {
      std::cout<<"Driver: "<<driver->name<<'\n';
      std::cout<<"Class: "<<net_classes[net_driver_class(driver->num)].name<<'\n';
      NET_CHANNEL* channel=net_openchannel(driver->num,0);
      std::cout<<"Default local address as reported by Libnet driver: "<<net_getlocaladdress(channel)<<"\n\n";
      net_closechannel(channel);

      NET_CONN* listen=net_openconn(driver->num,""); // "" is default for server
      net_listen(listen);
      listens.push_back(listen);
    }
    NET_CONN* conn;
    std::vector<NET_CONN*>::size_type i=0;
    while (true) {
      while (true) {
        if (i==listens.size())
          i=0;
        conn=net_poll_listen(listens[i]);
        if (conn!=0)
          break;
        ++i;
      }
      std::cout<<"Connected to "<<net_getpeer(conn)<<" using "<<initializeddrivers[i].name<<'\n';
      clients.push_back(conn);
      ++remoteplanes;
      if (static_cast<int>(clients.size())==maxremoteplanes ||
          static_cast<int>(clients.size())==maxplanes-1)
        break;
      std::cout<<"Enter an empty line to listen for another player:\n";
      std::string line;
      getline(std::cin,line);
      if (!line.empty())
        break;;
    }
    for_each(listens.begin(),listens.end(),net_closeconn);

    send_receive=new Send(clients);
  }
  else {
    while (true) {
      for (std::vector<NET_DRIVERNAME>::size_type i=0;i<detecteddrivers.size();++i)
        std::cout<<i<<": "<<detecteddrivers[i].name<<" ("<<net_classes[net_driver_class(detecteddrivers[i].num)].name<<")\n";
      std::cout<<"Choose driver (0-"<<detecteddrivers.size()-1<<"): ";
      std::string line;
      getline(std::cin,line);
      std::stringstream interpreter;
      interpreter<<line;
      if ((interpreter>>choice) && (interpreter>>std::ws).eof() &&
          choice>=0 && choice<static_cast<int>(detecteddrivers.size()))
        break;
      else
        std::cout<<'\n';
    }
    net_initdrivers(Driverlist(detecteddrivers[choice].num));
    server=net_openconn(detecteddrivers[choice].num,0); // 0 is recommended for client
    if (address.empty()) {
      std::cout<<"Enter target address: ";
      getline(std::cin,address);
    }
    std::cout<<"Trying to connect to "<<address<<'\n';
    net_connect(server,address.c_str());
    int status;
    do
      status=net_poll_connect(server);
    while (status==0);
    if (status>0)
      std::cout<<"Connected to "<<net_getpeer(server)<<'\n';
    else
      throw sw_excep("Unable to connect to server");

    send_receive=new Receive(server);
  }

  send_receive->add(remoteplanes);
  send_receive->add(randv);
  send_receive->add(latency);
  send_receive->add(level);
  send_receive->add(version);
  send_receive->add(computerplayers);
  send_receive->add(fixedorder);

  delete send_receive;
  if (connmode==SERVER) {
    std::vector<int> positions;
    positions.reserve(remoteplanes+1);
    for (int i=0;i<=remoteplanes;++i)
      positions.push_back(i);
    if (!fixedorder)
      random_shuffle(positions.begin(),positions.end());
    playerindex=positions[0];
    int i=1;
    const std::list<NET_CONN*>::const_iterator end=clients.end();
    for (std::list<NET_CONN*>::const_iterator iter=clients.begin();iter!=end;++iter,++i) {
      short data=tos(static_cast<short>(positions[i]));
      net_send_rdm(*iter,&data,sizeof(data));
      conntoposition.insert(std::pair<NET_CONN*,int>(*iter,positions[i]));
    }
  }
  else {
    short data;
    while (net_receive_rdm(server,&data,sizeof(data))==0);
    playerindex=tos(data);
  }
}

void deinitnetwork()
{
  if (connmode==SERVER) {
    Send* send=new Send(clients);
    short data=KEY_DISCONN;
    for (int i=0;i<remoteplanes;++i)
      send->add(data);
    delete send;
    const std::list<NET_CONN*>::const_iterator end=clients.end();
    for (std::list<NET_CONN*>::const_iterator iter=clients.begin();iter!=end;++iter)
      net_closeconn(*iter);
    clients.clear();
    conntoposition.clear();
  }
  else
    if (server!=0) {
      short data=tos(KEY_DISCONN);
      net_send_rdm(server,&data,sizeof(data));
      net_closeconn(server);
      server=0;
    }
  net_shutdown();
}

void putremotekeys(int keys)
{
  positiontokeys.clear();
  if (connmode==SERVER) {
    positiontokeys.insert(std::pair<int,int>(playerindex,keys));
    std::list<NET_CONN*> unansweredclients=clients;
    while (!unansweredclients.empty()) {
      const std::list<NET_CONN*>::const_iterator end=unansweredclients.end();
      for (std::list<NET_CONN*>::iterator iter=unansweredclients.begin();iter!=end;) {
        short data;
        if (net_receive_rdm(*iter,&data,sizeof(data))!=0) {
          data=tos(data);
          if ((data&KEY_DISCONN)!=0) {
            net_closeconn(*iter);
            clients.remove(*iter);
            speedtick=0;
          }
          positiontokeys.insert(std::pair<int,int>(conntoposition[*iter],data));
          iter=unansweredclients.erase(iter);
        }
        else
          ++iter;
      }
    }
    const std::list<NET_CONN*>::const_iterator end1=clients.end();
    for (std::list<NET_CONN*>::iterator conn=clients.begin();conn!=end1;++conn) {
      int position=conntoposition[*conn];
      Send* send=new Send(*conn);;
      const std::map<int,int>::const_iterator end=positiontokeys.end();
      for (std::map<int,int>::iterator pair=positiontokeys.begin();pair!=end;++pair)
        if (position!=pair->first)
          send->add(pair->second);
      delete send;
    }
  }
  else
    if (server!=0) {
      short data=tos(static_cast<short>(keys));
      net_send_rdm(server,&data,sizeof(data));
      bool serverclosed=true;
      Receive* receive=new Receive(server);
      int plane=0,position=0;
      for (;plane<remoteplanes;++position)
        if (position!=playerindex && vacantpositions.find(position)==vacantpositions.end()) {
          int keys;
          receive->add(keys);
          if ((keys&KEY_DISCONN)!=0) {
            vacantpositions.insert(position);
            speedtick=0;
          }
          else
            serverclosed=false;
          positiontokeys.insert(std::pair<int,int>(position,keys));
          ++plane;
        }
      delete receive;
      if (serverclosed) {
        net_closeconn(server);
        server=0;
      }
    }
}

int getremotekeys(int position)
{
  const std::map<int,int>::iterator iter=positiontokeys.find(position);
  if (iter!=positiontokeys.end())
    return iter->second;
  else
    return KEY_DISCONN;
}

#endif /* SOPWITH_LIBNET_NETWORK_H */
