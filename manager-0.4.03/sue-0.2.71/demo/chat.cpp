// +-------------------------------------------------------------------------+
// |              (S)imple (U)nix (E)vents vers. 0.2.71                      |
// | Copyright (c) Andrey Vikt. Stolyarov <crocodil_AT_croco.net> 2003-2008. |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                 GNU LESSER GENERAL PUBLIC LICENSE, v. 2.1               |
// | as published by Free Software Foundation      (see the file LGPL.txt)   |
// |                                                                         |
// | Please visit http://www.croco.net/software/sue for a fresh copy of Sue. |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




/*
  This program is a very simple TCP 'chat room' server. 
  It listens for TCP connections at the port 6666. You 
  can connect the server using, e.g., something like

            telnet 0 6666
  
  then enter your name and enjoy the chat. Unlimited 
  number of clients is allowed (in fact, the number is 
  limited by open descriptors limits).

  If you remain silent for 60 seconds, you get disconnected  
  (that is, "timed out").
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sue_sel.hpp"
#include "sue_tcps.hpp"

const int the_server_port = 6666;
const int the_server_timeout = 60;



class ChatServerSession : public SUETcpServerSession {
    char *name;
    class ChatServer *the_server;
public:
    ChatServerSession(int a_fd, int a_timeout, 
                      SUEEventSelector *a_selector,
                      ChatServer *a_server);

    ~ChatServerSession();

      // this function must be overriden in every application, 
      // it is the place to implement the main functionality of the server
    virtual void HandleNewInput();

      // when the session is closed for whatever reason, we'd like to 
      // notify all the remaining clients that one of them is gone
    virtual void TcpServerSessionShutdownHook(); 
      
      // we want to notify the user (and all the clients as well) that 
      // she is timed out; so we override this function. If we had left
      // it untouched, it would simply shut the session down saying nothing.
    virtual void HandleSessionTimeout();

      // this function is called by the server when it is necessary to send
      // some text to the client. It is just a piece of internals of
      // the particular application (chat room), not the SUE library.
    void Send(const char *message);

};



class ChatServer : public SUETcpServer {
    struct Item {
        ChatServerSession *sess;
        Item *next;
    }; 
    Item *first;

    int timeout;
public:
    ChatServer(int a_port, int a_timeout);
    ~ChatServer();

      // this is the only one we really MUST override in a child of
      // the SUETcpServer class. It must create another ChatServerSession
      // object.
    virtual SUETcpServerSession* SpawnSession(int newsessionfd);

      // all the rest appear to be the chat room implementation
    void SendMessage(const char *name, const char *message);
    void SendEvent(const char *name, const char *event);
    void ExcludeSession(ChatServerSession *sess);
private:
    void Send(const char *msg);
};











ChatServerSession::ChatServerSession(int a_fd, int a_timeout, 
	      SUEEventSelector *a_selector,
	      ChatServer *a_server)
: SUETcpServerSession(a_fd, a_timeout, a_selector, a_server, 
		    "Please enter your name: ")
{ 
    name = 0; 
    the_server = a_server; 

      // we want to time out the users who don't type anything in
    inputresetstimeout = true;
      // when a user gets a message (as opposit to _sending_ a message),
      // it doesn't affect her idle time counter
    outputresetstimeout = false;
}

ChatServerSession::~ChatServerSession() 
{ 
    if(name) 
        delete[] name; 
}

void ChatServerSession::HandleNewInput() 
{   
    SUEBuffer ln;
    while(inputbuffer.ReadLine(ln)) {
        if(!name) {
            if(ln.Length()<3) { // 3 is for one char and <CR><LF>
                outputbuffer.AddString("Name too short.\n"
                                       "Please enter your name: ");
            } else {
	        name = new char[ln.Length()+1];
	        strncpy(name, ln.GetBuffer(), ln.Length());
                name[ln.Length()] = 0;
	        the_server->SendEvent(name, "entered the chat room");
            }
        } else {
	    the_server->SendMessage(name, ln.GetBuffer());
        }
    }
}

void ChatServerSession::HandleSessionTimeout() 
{
    // NOTE the text "timed out" will be seen by all the clients
    // _including_ the one timed out, while the text
    // "left the chat room" will be seen by all the clients
    // _except_ the one timed out. This is because the first
    // one is sent before GracefulShutdown() is called, and the
    // former one is sent from the shutdown hook, that is, 
    // _after_ the session is closed.
    the_server->SendEvent(name, "timed out");
    GracefulShutdown();
}

void ChatServerSession::TcpServerSessionShutdownHook() 
{
    the_server->ExcludeSession(this);
    if(name) 
        the_server->SendEvent(name, "left the chat room");
}

void ChatServerSession::Send(const char *message)
{
    outputbuffer.AddString(message);
}




ChatServer::ChatServer(int a_port, int a_timeout)
    : SUETcpServer("0.0.0.0", a_port)
{
    first = 0;
    timeout = a_timeout;
}

ChatServer::~ChatServer()
{
    while(first) {
        Item* tmp = first;
        first = first->next;
        delete tmp;
    }
}

SUETcpServerSession* ChatServer::SpawnSession(int newsessionfd)
{
    Item *tmp = new Item;
    tmp->next = first;
    tmp->sess = new ChatServerSession(newsessionfd, timeout, selector, this);
    first = tmp;
    return first->sess;
}



void ChatServer::SendMessage(const char *name, const char *message)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "<%s> %s\n", name, message);
    Send(buf);
}

void ChatServer::SendEvent(const char *name, const char *event)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "* %s %s\n", name, event);
    Send(buf);
}

void ChatServer::ExcludeSession(ChatServerSession *sess)
{
    Item **tmp = &first;
    while(*tmp && (*tmp)->sess != sess) tmp = &((*tmp)->next);
    if(!*tmp) return;
    Item *to_del = *tmp;
    *tmp = (*tmp)->next;
    delete to_del;
}

void ChatServer::Send(const char *msg)
{
    printf("[chat] %s", msg);
    for(Item *tmp = first; tmp; tmp=tmp->next)
        tmp->sess->Send(msg);
}

int main(int argc, char **argv)
{
    SUEEventSelector selector;
    ChatServer serv(the_server_port, the_server_timeout);
    if(serv.Up(&selector)) { 
        printf("[chat] Listening port %d\n", the_server_port);
    } else {
        printf("Failed to bring the server up, exiting...\n");
        exit(1);
    }
    selector.Go();
    return 0;
}
