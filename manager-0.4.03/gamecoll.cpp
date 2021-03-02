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




#include "mgame.hpp"
#include "gamecoll.hpp"


GameCollection::GameCollection()
{
    first = 0;
    sequence = 1;
}

GameCollection::~GameCollection()
{
    while(first) {
        delete first->game;
        Item *tmp = first;
        first = tmp->next;
        delete tmp;
    }
}

AbstractGameSession* GameCollection::Create(PlayingClient *client, 
                                            const char *cmdparm)
{
    Item *tmp = new Item;
    ManagerGame *mgame = new ManagerGame(sequence++);
    tmp->game = mgame;
    tmp->next = first;
    first = tmp;
    return mgame->Join(client);
}

AbstractGameSession* GameCollection::Join(PlayingClient *client, int gm)
{
    for(Item* tmp=first; tmp; tmp=tmp->next) {
        if(tmp->game->GetSeqnum()==gm) {
            ManagerGame *mgame = static_cast<ManagerGame*>(tmp->game);
            return mgame->Join(client);
        }
    }
    return 0;
}

AbstractGame* GameCollection::GetGame(int gameid)
{
    for(Item* tmp=first; tmp; tmp=tmp->next) {
        if(tmp->game->GetSeqnum()==gameid) 
            return tmp->game;
    }
    return 0;
}

void GameCollection::RemoveZombies()
{
    Item **cur = &first;
    while(*cur) {
        if((*cur)->game->ZombieState()) {
            Item *tmp = *cur;
            *cur = (*cur)->next;
            delete tmp->game;
            delete tmp;
        } else {
            cur = &((*cur)->next);
        }
    }
}

int GameCollection::GameCount() const
{
    int ret = 0;
    for(Item* tmp=first; tmp; tmp=tmp->next) {
        ret++;
    }
    return ret;
}

