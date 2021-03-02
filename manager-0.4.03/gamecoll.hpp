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




#ifndef GAMECOLL_HPP_SENTRY
#define GAMECOLL_HPP_SENTRY

#include "session.hpp"

class GameCollection {
    struct Item {
        AbstractGame *game;
        Item *next;
    };
    Item *first;
    int sequence;
public:
    GameCollection();
    ~GameCollection();

    AbstractGameSession* Create(PlayingClient *a_client, const char *gtype);
    AbstractGameSession* Join(PlayingClient *a_client, int gameid);

    void RemoveZombies();
    
    AbstractGame *GetGame(int gameid);

    int GameCount() const;
};


#endif
