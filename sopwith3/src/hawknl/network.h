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
#ifndef SOPWITH_HAWKNL_NETWORK_H
#define SOPWITH_HAWKNL_NETWORK_H

#include <list>
#include <map>
#include <set>
#include <nl.h>
#include "../resource.h"
#include "../graphics.h"
#include "../keyboard.h"
#include "../sopwith.h"
#include "../sw_excep.h"

namespace {
  void error()
  {
    std::stringstream exception;
    NLenum error=nlGetError();
    if (error==NL_SYSTEM_ERROR)
      exception << "HawkNL: System error: " << nlGetSystemErrorStr(nlGetSystemError());
    else
      exception << "HawkNL: " << nlGetErrorStr(error);
    throw sw_excep(exception);
  }

  class Socket {
  public:
    Socket()
    : socket(NL_INVALID)
    {
    }
    ~Socket()
    {
      if (socket!=NL_INVALID)
        nlClose(socket);
    }
    NLint read(void* buffer,int nbytes)
    {
      static char prebuffer[NL_MAX_PACKET_LENGTH];
      for (int i=0;i<nbytes;) {
        while (ss.peek()==EOF) {
          NLint amount=nlRead(socket,reinterpret_cast<NLvoid*>(prebuffer),NL_MAX_PACKET_LENGTH);
          if (amount==NL_INVALID)
            return NL_INVALID;
          else {
            ss.write(prebuffer,amount);
          }
        }
        i+=ss.readsome(reinterpret_cast<char*>(buffer)+i,nbytes-i); 
      }
      return nbytes;
    }
    NLsocket socket;
  private:
    std::stringstream ss;
  };
}

bool networkavailable=true;
Socket* server;
std::list<Socket*> clients;
std::map<Socket*,int> conntoposition;
std::map<int,int> positiontokeys;
std::set<int> vacantpositions;

class Send_Receive {
public:
  virtual ~Send_Receive() {}
  virtual void add(bool& data)=0;
  virtual void add(short& data)=0;
  virtual void add(int& data)=0;
};

class Send : public Send_Receive {
public:
  Send(Socket* conn)
  {
    conns_.push_back(conn);
  }
  Send(const std::list<Socket*>& conns) : conns_(conns) {}
  virtual void add(bool& data)
  {
    short temp=nlSwaps(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual void add(short& data)
  {
    short temp=nlSwaps(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual void add(int& data)
  {
    short temp=nlSwaps(data);
    out.write(reinterpret_cast<char*>(&temp),sizeof(temp));
  }
  virtual ~Send()
  {
    std::string s=out.str();
    const std::list<Socket*>::const_iterator end=conns_.end();
    for (std::list<Socket*>::const_iterator iter=conns_.begin();iter!=end;++iter)
      nlWrite((*iter)->socket,s.data(),s.length());
  }
private:
  std::list<Socket*> conns_;
  std::stringstream out;
};

class Receive : public Send_Receive {
public:
  Receive(Socket* conn) : conn_(conn)
  {
  }
  virtual void add(bool& data)
  {
    unsigned char buffer[sizeof(short)];
    conn_->read(buffer,sizeof(short));
    data=nlSwaps(*reinterpret_cast<short*>(buffer));
  }
  virtual void add(short& data)
  {
    unsigned char buffer[sizeof(short)];
    conn_->read(buffer,sizeof(short));
    data=nlSwaps(*reinterpret_cast<short*>(buffer));
  }
  virtual void add(int& data)
  {
    unsigned char buffer[sizeof(short)];
    conn_->read(buffer,sizeof(short));
    data=nlSwaps(*reinterpret_cast<short*>(buffer));
  }
  ~Receive()
  {
  }
private:
  Socket* conn_;
};

void initnetwork()
{
  Resource graphicsobject(textmode,graphicsmode);
  Resource keyboardobject(deinitkeyboard,initkeyboard);
  if (!nlInit())
    error();
  if (!nlSelectNetwork(NL_IP))
    error();
  if (!nlEnable(NL_BLOCKING_IO))
    error();
  if (!nlEnable(NL_TCP_NO_DELAY))
    error();

  Send_Receive* send_receive;

  if (connmode==SERVER) {
    for (;static_cast<int>(clients.size())<maxremoteplanes && static_cast<int>(clients.size())<maxplanes-1;++remoteplanes) {
      std::cout<<"Enter port number to listen for player or anything else to continue: ";
      std::string line;
      getline(std::cin,line);
      std::stringstream interpreter;
      interpreter<<line;
      NLushort serverport;
      if ((interpreter>>serverport) && (interpreter>>std::ws).eof() &&
          std::cin) {
        NLsocket listen;
        if ((listen=nlOpen(serverport,NL_RELIABLE_PACKETS))==NL_INVALID)
          error();
        if (!nlListen(listen)) {
          nlClose(listen);
          error();
        }
        Socket* conn=new Socket;
        while ((conn->socket=nlAcceptConnection(listen))==NL_INVALID) {
          if (nlGetError()!=NL_NO_PENDING) {
            nlClose(listen);
            error();
          }
        }

        NLaddress address;
        nlGetRemoteAddr(conn->socket,&address);
        NLchar string[NL_MAX_STRING_LENGTH];
        nlAddrToString(&address,string);
        std::cout<<"Connected to "<<string<<'\n';

        nlClose(listen);
        clients.push_back(conn);
      }
      else
        break;
    }

    send_receive=new Send(clients);
  }
  else {
    server=new Socket;
    server->socket=nlOpen(0,NL_RELIABLE_PACKETS); // 0 is recommended for client
    if (address.empty()) {
      std::cout<<"Enter target address: ";
      getline(std::cin,address);
    }
    NLaddress addr;
    if (!nlGetAddrFromName(address.c_str(),&addr))
      error();
    std::cout<<"Enter port: ";
    NLushort serverport;
    std::cin>>serverport;
    if (!nlSetAddrPort(&addr,serverport))
      error();
    std::cout<<"Trying to connect to "<<address<<'\n';
    if (nlConnect(server->socket,&addr)) {
      NLaddress address;
      nlGetRemoteAddr(server->socket,&address);
      NLchar string[NL_MAX_STRING_LENGTH];
      nlAddrToString(&address,string);
      std::cout<<"Connected to "<<string<<'\n';
    }  
    else
      error();

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
    const std::list<Socket*>::const_iterator end=clients.end();
    for (std::list<Socket*>::const_iterator iter=clients.begin();iter!=end;++iter,++i) {
      short data=nlSwaps(static_cast<short>(positions[i]));
      nlWrite((*iter)->socket,&data,sizeof(data));
      conntoposition.insert(std::pair<Socket*,int>(*iter,positions[i]));
    }
  }
  else {
    short data;
    server->read(&data,sizeof(data));
    playerindex=nlSwaps(data);
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
    const std::list<Socket*>::const_iterator end=clients.end();
    for (std::list<Socket*>::const_iterator iter=clients.begin();iter!=end;++iter)
      delete *iter;
    clients.clear();
    conntoposition.clear();
  }
  else
    if (server!=0) {
      short data=nlSwaps(KEY_DISCONN);
      nlWrite(server->socket,&data,sizeof(data));
      delete server;
      server=0;
    }
  nlShutdown();
}

void putremotekeys(int keys)
{
  positiontokeys.clear();
  if (connmode==SERVER) {
    positiontokeys.insert(std::pair<int,int>(playerindex,keys));
    std::list<Socket*> unansweredclients=clients;
    while (!unansweredclients.empty()) {
      const std::list<Socket*>::const_iterator end=unansweredclients.end();
      for (std::list<Socket*>::iterator iter=unansweredclients.begin();iter!=end;) {
        short data;
        if ((*iter)->read(&data,sizeof(data))!=NL_INVALID) {
          data=nlSwaps(data);
          if ((data&KEY_DISCONN)!=0) {
            delete *iter;
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
    const std::list<Socket*>::const_iterator end1=clients.end();
    for (std::list<Socket*>::iterator conn=clients.begin();conn!=end1;++conn) {
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
      short data=nlSwaps(static_cast<short>(keys));
      nlWrite(server->socket,&data,sizeof(data));
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
        delete server;
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

#endif /* SOPWITH_HAWKNL_NETWORK_H */
