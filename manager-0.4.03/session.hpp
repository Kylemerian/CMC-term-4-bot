// +-------------------------------------------------------------------------+
// |                   Manager game server, vers. 0.4.01                     |
// |    Copyright (c) Andrey Vikt. Stolyarov <avst_AT_cs.msu.ru> 2004-2006   |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                     GNU GENERAL PUBLIC LICENSE, v.2                     |
// | as published by Free Software Foundation      (see the file COPYING)    |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#ifndef SESSION_HPP_SENTRY
#define SESSION_HPP_SENTRY

class PlayingClient {
public:
    PlayingClient() {}
    virtual ~PlayingClient() {}

    virtual void Print(const char *) = 0;
    virtual void Broadcast(const char *) = 0;

    virtual const char *GetName() const = 0;
};


class AbstractGameSession {
protected:
    PlayingClient *the_client;
public:
    AbstractGameSession(PlayingClient *a_client) : the_client(a_client) {}
    virtual ~AbstractGameSession() {}

    virtual void HandleCommand(const char *cmd) = 0;
    virtual bool ChatAccepted() const = 0;
    virtual int GameId() const = 0;

    virtual const char *GetStatus() const = 0;
    virtual bool ZombieState() const = 0;

    void SendMessage(const char *msg) const 
        { the_client->Print(msg); }

};

class AbstractGame {
    int seqnum;
public:
    AbstractGame(int n) : seqnum(n) {}
    virtual ~AbstractGame() {}

    virtual bool ZombieState() const = 0;

    int GetSeqnum() const { return seqnum; }
};



#endif
