// +-------------------------------------------------------------------------+
// |                   Manager game server, vers. 0.4.02                     |
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




#ifndef MGAME_HPP_SENTRY
#define MGAME_HPP_SENTRY

#include "scriptpp/scrvar.hpp"
#include "session.hpp"

#define MAX_PLANTS 100

class ManagerGame : public AbstractGame {
    enum game_status { 
        gs_notstarted,
        gs_playing,
        gs_finished,
        gs_aborted
    } state; 

    ScriptVariable status_message;
    class Market *market;

    struct Item {
        Item *next;
        class ManagerGameSession *sess;
    };
    Item *first;

public:
    ManagerGame(int seqn);
    virtual ~ManagerGame();

    /* from AbstractGame */
    virtual bool ZombieState() const { return (first == 0); }

    AbstractGameSession* Join(PlayingClient *client); 

    int GetNumplayers() const; 
    int GetAlivePlayers() const; 

    void Start();
    const char *GetStatus() const;
    const char *GameStatusMessage() const;

    void RemoveSession(class ManagerGameSession *sess);

    bool IsStarted() const { return state != gs_notstarted; }
    bool IsFinished() const { return state == gs_finished; }
    void CheckEndTurn();
    void DoEndTurn();

    const Market* GetMarket() const { return market; }

    void SendMeInfo(class ManagerGameSession *sess);

    void Broadcast(const char *msg) const;
};



class ManagerGameSession : public AbstractGameSession {
    ManagerGame *the_game;
    bool is_creator;
    bool is_turn_ended;
    bool is_spectator;
    bool wishes_to_quit;
    enum { chat_on, chat_off, chat_notingame } chat_mode;

    int raw_request;
    int raw_request_price;
    int prod_request;
    int prod_request_price;
    int creation_request;

    int money;
    int raw;
    int prod;

    enum PlantType { 
        plant_none = 0, 
        plant_built = 1, 
        plant_abuilt = 2, 
        plant_ordinary = 3,
        plant_reconstructed = 4,
        plant_automatic = 5
    };

    struct Plant {
        PlantType t;
        int month_left;
    } plants[MAX_PLANTS];

public:
    ManagerGameSession(ManagerGame *master, PlayingClient *cli, bool cre);
    virtual ~ManagerGameSession();

    virtual bool ZombieState() const { return wishes_to_quit; }

    void ForgetGame() { the_game = 0; }

    bool IsCreator() const { return is_creator; }
    bool IsSpectator() const { return is_spectator; }

    
    bool IsTurnEnded() const { return is_turn_ended; }
    void TurnEnd(); // called by ManagerGame when everyone are ready
    void BuyRaw(int amount, int price);
    void SellProd(int amount, int price);

    void GetActives(int &r, int &p, int &m) const
        { r = raw; p = prod; m = money; }
    void GetPlants(int &ordinary, int &autopl) const;
    void GetTradeRequests(int &r, int &rp, int &p, int &pp) const;

    const char *GetName() const { return the_client->GetName(); }

private: 
    /* from AbstractGameSession */
    virtual bool ChatAccepted() const;
    virtual void HandleCommand(const char *cmd); 
    virtual int GameId() const { return the_game->GetSeqnum(); }

    mutable ScriptVariable status_string;
    virtual const char *GetStatus() const;

    void SendPrompt();
    void ClearRequests();

    void BuyArrange(const ScriptVariable &n, const ScriptVariable &pr);
    void SellArrange(const ScriptVariable &n, const ScriptVariable &pr);
    void ProdArrange(const ScriptVariable &n);
    void BuildArrange(bool auto_plant);
    void UpgradeArrange();
};


#endif
