// +-------------------------------------------------------------------------+
// |                   Manager game server, vers. 0.4.03                     |
// |    Copyright (c) Andrey Vikt. Stolyarov <avst_AT_cs.msu.ru> 2004-2011   |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                     GNU GENERAL PUBLIC LICENSE, v.2                     |
// | as published by Free Software Foundation      (see the file COPYING)    |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

#include "sue/sue_sel.hpp"
#include "sue/sue_tcps.hpp"

#include "scriptpp/scrvar.hpp"
#include "scriptpp/scrvect.hpp"

#include "session.hpp"
#include "gamecoll.hpp"

/*const*/ int the_server_port = 4774;
const int the_server_timeout = 3600;
const int max_name_length = 16;


class ChatServer;

class ChatServerSession : public SUETcpServerSession, public PlayingClient {
    ChatServer *the_server;
    char *name;
    AbstractGameSession *session;
public:
    ChatServerSession(int a_fd, int a_timeout, 
                      SUEEventSelector *a_selector,
                      ChatServer *a_server);
    ~ChatServerSession();

    /* from TcpServerSession */
    virtual void HandleNewInput();
    virtual void TcpServerSessionShutdownHook(); 
    virtual void HandleSessionTimeout();

    /* from PlayingClient */
    virtual void Print(const char *msg) { Send(msg); }
    virtual void Broadcast(const char *);
    virtual const char *GetName() const { return name; }

    void Send(const char *message);

    const char *GetStatus() const 
        { return session ? session->GetStatus() : "[relaxing]"; }

    void ChatSend(const char *message) 
        { if(!session||session->ChatAccepted()) Send(message); }

    int GameId() const { return session ? session->GameId() : 0; }

#if 0
    const char *GetStatus() const 
        { return session ? session->GetStatus() : "[relaxing]"; }

private:
    virtual void SessionDead(); 
    virtual void WinnerNotification(int gmnum);
    virtual void SendMessage(const char *msg) { Send(msg); }
#endif
    void ProcessCommand(const char *);
};



class ChatServer : public SUETcpServer {
    struct Item {
        ChatServerSession *sess;
        Item *next;
    }; 
    Item *first;

    int timeout;

    GameCollection *the_collection;

public:
    ChatServer(int a_port, int a_timeout, GameCollection *coll);
    ~ChatServer();

    virtual SUETcpServerSession* SpawnSession(int newsessionfd);

    void SendMessage(const char *name, const char *message);
    void SendEvent(const char *name, const char *event);
    void Broadcast(const char *msg) { Send(msg, true); }
    void ExcludeSession(ChatServerSession *sess);

    bool IsNameAvailable(const char *name) const;
    void SendMeList(ChatServerSession *sess) const;
    ChatServerSession* FindByName(const char *name) const;

    AbstractGameSession *CreateGame(ChatServerSession *sess,
                                    const char *gametype)
        { return the_collection->Create(sess, gametype); }
    AbstractGameSession *JoinGame(ChatServerSession *sess,
                                  int gameid)
        { return the_collection->Join(sess, gameid); }


    void RemoveZombieGames() const { the_collection->RemoveZombies(); }
private:
    void Send(const char *msg, bool urgent = false);
};





ChatServerSession::ChatServerSession(int a_fd, int a_timeout, 
	      SUEEventSelector *a_selector,
	      ChatServer *a_server)
: SUETcpServerSession(a_fd, a_timeout, a_selector, a_server, 
		    "Please enter your name: ")
{ 
    name = 0; 
    session = 0;
    the_server = a_server; 

      // we want to time out the users who don't type anything in
    inputresetstimeout = true;
      // when a user gets a message (as opposit to _sending_ a message),
      // it doesn't affect her idle time counter
    outputresetstimeout = false;
}

ChatServerSession::~ChatServerSession() 
{ 
    if(session) 
        delete session;
    if(name) 
        delete[] name; 
}

static bool check_login_name(const char *c)
{
    for(int i=0; c[i]; i++) {
        if(!isalnum(c[i])) return false;
        if(isspace(c[i])) return false;
        if(((unsigned char)(c[i]))>127) return false;
    }
    return true;
}

void ChatServerSession::HandleNewInput() 
{   
    SUEBuffer ln;
    while(inputbuffer.ReadLine(ln)) {
        if(!name) {
            if(ln.Length()<3) { // 3 is for one char, <LF> and <0>
                outputbuffer.AddString("%- Name too short.\n"
                                       "Please enter your name: ");
            } else
            if(ln.Length()>max_name_length+2) { // 2 is for <LF> and <0>
                outputbuffer.AddString("%- Name too long.\n"
                                       "Please enter your name: ");
            } else 
            {
	        char *str = new char[ln.Length()+1];
	        strncpy(str, ln.GetBuffer(), ln.Length());
                str[ln.Length()] = 0;
		if(!check_login_name(str)) {
                    outputbuffer.AddString("%- Bad symbols in the name "
                                   "(only letters and digits are allowed)\n"
                                           "Please enter your name: ");
                    delete[] str;
		} else
                if(the_server->IsNameAvailable(str)) {
                    name = str;
	            the_server->SendEvent(name, "has entered the chat room");
                    outputbuffer.AddString("% Type .help for help\n");
                } else {
                    outputbuffer.AddString("%- Name is not available "
                                           "(someone is already using it)\n"
                                           "Please enter your name: ");
                    delete[] str;
                }
            }
        } else 
        if(session && ln[0]!='.') {
            session->HandleCommand(ln.GetBuffer()); 
        } else {
            // Since we've made a successful ReadLine, there's at least
            // one byte in the buffer ('\n')
            switch(ln[0]) {
                case '\r':
                case '\n':
                case '\0':
                    Send("% Your name is ");
                    Send(name);
                    Send("\n% You are in the chat room. "
                         "Type .help to see the list of commands\n"
                         "% Type something else than a command to talk\n");
                    break;
                case '.':
                    ProcessCommand(ln.GetBuffer());
                    break;                    
                default:
	            the_server->SendMessage(name, ln.GetBuffer());
            }
        }
    }
    if(session && session->ZombieState()) {
        delete session;
        session = 0;
        the_server->SendEvent(name, 
            "has left a game and returned to the chat room");
        the_server->RemoveZombieGames();
    }
}

void ChatServerSession::HandleSessionTimeout() 
{
    the_server->SendEvent(name, "timed out");
    GracefulShutdown();
}

void ChatServerSession::TcpServerSessionShutdownHook() 
{
    the_server->ExcludeSession(this);
    if(name) 
        the_server->SendEvent(name, "has left the chat room");
}

void ChatServerSession::Broadcast(const char *message)
{
    the_server->Broadcast(message);
}

void ChatServerSession::Send(const char *message)
{
    if(name) outputbuffer.AddString(message);
}


#define MUST_BE_RELAXING \
    if(session) {\
        outputbuffer.AddString("%- You can't do that while playing a game\n");\
        return;\
    }

void ChatServerSession::ProcessCommand(const char *cmd)
{
    ScriptVector cmdline(cmd);
    if(cmdline[0]==".help") {
        outputbuffer.AddString(
        "% .who                    - list who's on\n"
        "% .tell <nick> <message>  - send a private message\n"
        "% .say <message>          - send a public message (or just type it)\n"
        "% .create                 - create new game\n"
        "% .join N                 - join the game #N\n"
        "% .join <nick>            - join the game where <nick> plays\n"
        "% .quit                   - quits the server\n"
        "% .help                   - prints this help\n"
        ); 
    } else
    if(cmdline[0]==".quit") {
        outputbuffer.AddString("% Bye-bye\n");
#if 0
        if(session) {
            int gmid = session->GameId();
            AbstractGame* game = the_collection->GetGame(gmid);
            if(game)
                game->PlayerQuit(session);
        }
        if(session) {
            session->ClientDead();
            session = 0;
        }
#endif
        GracefulShutdown(); 
    } else 
    if(cmdline[0]==".create") {
        MUST_BE_RELAXING
        session = the_server->CreateGame(this, cmdline[1].c_str());
        if(!session) 
            outputbuffer.AddString("%- Couldn't create a game\n");
        else {
	    ScriptVariable sv(30, "has created the game #%d", 
                                  session->GameId());
            the_server->SendEvent(name, sv.c_str());
	}
    } else 
    if(cmdline[0]==".join") {
        MUST_BE_RELAXING
        long gmid;
        if(!cmdline[1].GetLong(gmid)) {
            ChatServerSession *nickowner = 
                the_server->FindByName(cmdline[1].c_str());
            if(!nickowner) {
                outputbuffer.AddString("%- No such nick\n");
                return;
            }
            gmid = nickowner->GameId();
            if(!gmid) {
                outputbuffer.AddString("%- That one is not playing now\n");
                return;
            }
        }
        session = the_server->JoinGame(this, gmid);
        if(!session) 
            outputbuffer.AddString("%- Couldn't join the game\n");
        else 
            the_server->SendEvent(name, "joined a game");
    } else 
    if(cmdline[0]==".who") {
        the_server->SendMeList(this);
    } else 
    if(cmdline[0]==".tell") {
        ChatServerSession *to = the_server->FindByName(cmdline[1].c_str());
        if(!to) {
            outputbuffer.AddString("%- No such nick \n");
            return;
        }
        ScriptVariable msg("* ");
        msg+=name;
        msg+=" tells you: ";
        for(int i=2; i<cmdline.Length(); i++) {
            msg += cmdline[i];
            msg += " ";
        }
        msg += "\n";
        to->Send(msg.c_str());
        outputbuffer.AddString("% OK\n");
    } else 
    if(cmdline[0]==".say") {
        if(session && !session->ChatAccepted()) {
            outputbuffer.AddString("%- Enable global chat to do this\n");
            return;
        }
        ScriptVariable msg;
        for(int i=1; i<cmdline.Length(); i++) {
            msg += cmdline[i];
            msg += " ";
        }
        the_server->SendMessage(name, msg.c_str());
        outputbuffer.AddString("% OK\n");
    } else 
    {
        outputbuffer.AddString("%- Unknown command [");
        outputbuffer.AddString(cmdline[0].c_str());
        outputbuffer.AddString("]\n");
    }
    if(session && session->ZombieState()) {
        delete session;
        session = 0;
        the_server->SendEvent(name, 
            "has left a game and returned to the chat room");
    }
}
#undef MUST_BE_RELAXING







ChatServer::ChatServer(int a_port, int a_timeout, GameCollection *coll)
    : SUETcpServer("0.0.0.0", a_port)
{
    first = 0;
    timeout = a_timeout;
    the_collection = coll;
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

void ChatServer::Send(const char *msg, bool urgent)
{
    fprintf(stderr, "[chat] %s", msg);
    fflush(stdout);
    for(Item *tmp = first; tmp; tmp=tmp->next)
        if(urgent)
            tmp->sess->Send(msg);
        else 
            tmp->sess->ChatSend(msg);
}

bool ChatServer::IsNameAvailable(const char *name) const
{
    return !FindByName(name);
}

ChatServerSession* ChatServer::FindByName(const char *name) const
{
    for(Item *tmp = first; tmp; tmp=tmp->next) {
        const char *p = tmp->sess->GetName();
        if(p && strcmp(p, name)==0) return tmp->sess;
    }
    return 0;
}

void ChatServer::SendMeList(ChatServerSession *back) const
{
    int count = 0;
    for(Item *tmp = first; tmp; tmp=tmp->next) {
        ScriptVariable sv("% "); 
        const char *name = tmp->sess->GetName();
	if(!name) continue;
	sv += name;
        while(sv.Length()<max_name_length+10) sv += " ";
        sv += tmp->sess->GetStatus();
        sv += "\n";
        back->Send(sv.c_str());
        count++;
    }
    ScriptVariable sv(80, "%% %d players online. %d games are played\n",
                          count, the_collection->GameCount());
    back->Send(sv.c_str());
}


class SigtermHandler : public SUESignalHandler {
    SUEEventSelector *selector;
public:
    SigtermHandler(SUEEventSelector *a_sel)
      : SUESignalHandler(SIGINT) { selector = a_sel; }
    ~SigtermHandler() {}
    virtual void SignalHandle() { selector->Break(); }
};



int main(int argc, char **argv)
{
    try {
        if(argc>1) {
            the_server_port = atoi(argv[1]);
            if(!the_server_port) {
                fprintf(stderr, "Invalid port number\n");
                exit(1);
            }
        }
        SUEEventSelector selector;
        GameCollection collection;
        ChatServer serv(the_server_port, the_server_timeout, &collection);
        if(serv.Up(&selector)) { 
            fprintf(stderr, "[chat] Listening port %d\n", the_server_port);
        } else {
            fprintf(stderr, "Failed to bring the server up, exiting...\n");
            exit(1);
        }

        SigtermHandler term(&selector);
        selector.RegisterSignalHandler(&term);
        for(;;) {
            try {
                selector.Go();
                fprintf(stderr, "Main loop broken... why?\n");
                break;
            }
            catch(const char *str) {
                fprintf(stderr, "Exception: %s\n", str);
            }
        }
    }
    catch(const char *str) {
        fprintf(stderr, "Fatal: %s\n", str);
    }
    catch(...) {
        fprintf(stderr, "Unknown exception caught\n");
        return 1;
    } 
    return 0;
}
