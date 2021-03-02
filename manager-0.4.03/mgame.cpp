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




#include <stdlib.h>

#include "scriptpp/scrvect.hpp"
#include "stock.hpp"
#include "mgame.hpp"


// local class
class Market {
    int level;

    struct LevelParameters {
        int raw2;
        int raw_price;
        int prod2;
        int prod_price;
    };

    static const int change_level_table[5][5];
    static const LevelParameters level_parameters[5];

public:
    Market() { level = 3; }
    ~Market() {}

    int GetLevel() const { return level; }
    void ChangeLevel() {
        // make a random in the range from 1 thru 12
        int r = 1 + (int) (12.0*rand()/(RAND_MAX+1.0));
        int new_level = 0;
        while(r>0 && new_level<=5) {
            r -= change_level_table[level-1][new_level];
            new_level++;
        }
        if(r>0 || new_level<1 || new_level>5) 
            throw "BUG: Market::ChangeLevel:: something wrong";
        level = new_level;
    }
    void GetLevelParameters(int pl_num, 
                            int &raw_amount, int &min_raw_price,
                            int &prod_amount, int &max_prod_price) const
    {
        raw_amount     = (level_parameters[level-1].raw2*pl_num+1)/2;
        min_raw_price  = level_parameters[level-1].raw_price;
        prod_amount    = (level_parameters[level-1].prod2*pl_num+1)/2;
        max_prod_price = level_parameters[level-1].prod_price;
    }
};

const int Market::change_level_table[5][5] = 
    { { 4, 4, 2, 1, 1 },
      { 3, 4, 3, 1, 1 },
      { 1, 3, 4, 3, 1 },
      { 1, 1, 3, 4, 3 },
      { 1, 1, 2, 4, 4 }
    };

const Market::LevelParameters Market::level_parameters[5] = 
    { { 2, 800, 6, 6500 },
      { 3, 650, 5, 6000 },
      { 4, 500, 4, 5500 },
      { 5, 400, 3, 5000 },
      { 6, 300, 2, 4500 }
    };



ManagerGame::ManagerGame(int seqn)
    : AbstractGame(seqn), 
      status_message(20, "waiting #%d", seqn) 
{ 
    state = gs_notstarted; 
    market = new Market;
    first = 0;
}

ManagerGame::~ManagerGame()
{
    delete market;
    while(first) {
        /* in fact this should never happen, but let it be... */
        Item *tmp = first;
        first = tmp->next;
        tmp->sess->ForgetGame(); // don't call the dead game!
        delete tmp;
    }
}

AbstractGameSession* ManagerGame::Join(PlayingClient *cli)
{
    bool be_creator = (first == 0);
    ManagerGameSession *game = new ManagerGameSession(this, cli, be_creator);
    Item *tmp = new Item;
    tmp->sess = game;
    tmp->next = first;
    first = tmp;
    Broadcast(ScriptVariable(0, "@+ JOIN %s\n", cli->GetName()).c_str());
    return game;
}

void ManagerGame::Start()
{
    state = gs_playing;
    status_message = ScriptVariable(20, "playing #%d", GetSeqnum());
    Broadcast("& START\n");
}

const char* ManagerGame::GameStatusMessage() const
{
    switch(state) {
        case gs_notstarted:
            return "# The game is not started yet\n";
        case gs_playing:
            return "# The game is in progress\n";
        case gs_finished:
            return "# The game is finished, type `quit' to quit\n";
        case gs_aborted:
            return "# The game was aborted, type `quit' to quit\n";
        default:
            return "";
    }
}

const char* ManagerGame::GetStatus() const
{
    return status_message.c_str();
}

void ManagerGame::RemoveSession(ManagerGameSession *sess)
{
    bool creator_quit = sess->IsCreator();

    Item **cur = &first;
    while(*cur) {
        if((*cur)->sess == sess) {
            ScriptVariable msg(80,
                "@- LEFT %s\n# %s has left the game #%d\n", 
                sess->GetName(), sess->GetName(), GetSeqnum());
            Broadcast(msg.c_str());

            Item *tmp = *cur;
            *cur = (*cur)->next;
            delete tmp;
        } else {
            cur = &((*cur)->next);
        }
    }

    if(state == gs_playing) 
        CheckEndTurn();
    if(creator_quit) {
        Broadcast("# Creator left the game, type quit to leave the game\n"
                  "& ABORT\n");
        state = gs_aborted;
    }
}

int ManagerGame::GetNumplayers() const
{
    int c = 0;
    for(Item *iter = first; iter; iter = iter->next) {
        c++;
    }
    return c;
}

int ManagerGame::GetAlivePlayers() const
{
    int c = 0;
    for(Item *iter = first; iter; iter = iter->next) {
        ManagerGameSession *p = iter->sess;
        if(!p->IsSpectator() && !p->ZombieState()) c++;  
    }
    return c;
}

void ManagerGame::CheckEndTurn()
{
    bool ok = true;
    ScriptVariable still_thinking("# Still thinking: ");
    for(Item *iter = first; iter; iter = iter->next) {
        ManagerGameSession *p = iter->sess;
        if(!p->IsSpectator() && !p->IsTurnEnded()) {
            ok = false;
            still_thinking += p->GetName();
            still_thinking += " ";
        }
    }
    still_thinking += "\n";
    if(ok) {
        DoEndTurn();
    } else {
        Broadcast(still_thinking.c_str());
    }
}

void ManagerGame::DoEndTurn()
{
    Broadcast("# Trading results:\n");
    ScriptVariable msg(80, "# --------  %16s %10s %10s\n",
                           "name", "amount", "price");
    Broadcast(msg.c_str());
    int n = GetAlivePlayers();
    Stock raw_stock(n);
    Stock prod_stock(n);

    // fill bids for trade sessions
    for(Item *iter = first; iter; iter = iter->next) {
        ManagerGameSession *p = iter->sess;
        if(p->IsSpectator()) continue;
        int tr, trp, tp, tpp;
        p->GetTradeRequests(tr, trp, tp, tpp); 
        raw_stock.AddBid(p, tr, trp);
        prod_stock.AddBid(p, tp, tpp);
    }

    int raw, prod, skip;
    market->GetLevelParameters(n, raw, skip, prod, skip);
    raw_stock.Run(raw, true);
    prod_stock.Run(prod, false);

    void *id;
    int amount, price;
    while(raw_stock.GetWinner(id, amount, price)) {
        ((ManagerGameSession*)id)->BuyRaw(amount, price);
    }
    while(prod_stock.GetWinner(id, amount, price)) {
        ((ManagerGameSession*)id)->SellProd(amount, price);
    }

    // actually end turn
    for(Item *iter = first; iter; iter = iter->next) {
        ManagerGameSession *p = iter->sess;
        if(p->IsSpectator()) continue;
        p->TurnEnd();
    }

    // check if the game is over
    switch(GetAlivePlayers()) {
        case 0:
            Broadcast("& NOWINNER\n");
            state = gs_finished;
            return; 
        case 1: {
            ManagerGameSession *the_winner = 0;
            for(Item *iter = first; iter; iter = iter->next) {
                ManagerGameSession *p = iter->sess;
                if(!p->IsSpectator() && !p->ZombieState()) {
                    the_winner = p;
                }
            }
            ScriptVariable winmsg(0, "# %s is the winner of the game\n"
                                     "& WINNER %s\n",
                                     the_winner->GetName(),
                                     the_winner->GetName());
            for(Item *iter = first; iter; iter = iter->next) {
                ManagerGameSession *p = iter->sess;
                if(p == the_winner) {
                    p->SendMessage("# Congratulations, you win the game\n");
                    p->SendMessage("& YOU_WIN\n");
                } else {
                    p->SendMessage(winmsg.c_str());
                }
            }
            state = gs_finished;
            return;
        }
        default:
            ;
    }

    //
    market->ChangeLevel();
}

void ManagerGame::SendMeInfo(ManagerGameSession *sess)
{
    if(state == gs_playing)
    {
        ScriptVariable head(80, "%-7s %16s %4s %4s %8s %4s %4s\n", 
            "# -----", "Name", "Raw", "Prod", "Money", "Plants", "AutoPlants");
        sess->SendMessage(head.c_str());
    
        for(Item *iter = first; iter; iter = iter->next) {
            ManagerGameSession *p = iter->sess;
            if(p->IsSpectator()) continue;
            const char *name = p->GetName();
            int raw, prod, money;
            p->GetActives(raw, prod, money);
            int plant, autopl;
            p->GetPlants(plant, autopl);
            ScriptVariable info(80, "%-7s %16s %4d %4d %8d %4d %4d\n", 
                     "& INFO", name, raw, prod, money, plant, autopl);
            sess->SendMessage(info.c_str());
        }
    }
    sess->SendMessage("# -----\n");
    int alp = GetAlivePlayers();
    int ttp = GetNumplayers();
    ScriptVariable plz(80, "%-16s %4d %16s %4d\n", 
                       "& PLAYERS", alp, "WATCHERS", ttp - alp);
    sess->SendMessage(plz.c_str());
    sess->SendMessage("# -----\n");
}

void ManagerGame::Broadcast(const char *msg) const 
{
    for(Item *tmp = first; tmp; tmp=tmp->next) {
        tmp->sess->SendMessage(msg);
    }
}








ManagerGameSession::ManagerGameSession(ManagerGame *master, 
                                       PlayingClient *cli, 
                                       bool cre)
    : AbstractGameSession(cli) 
{ 
    the_game = master; 
    is_creator = cre;
    is_turn_ended = false;
    is_spectator = the_game->IsStarted();
    wishes_to_quit = false;
    chat_mode = chat_notingame;
    
    ClearRequests();
    money = 10000;
    raw = 2;
    prod = 2;
    for(int i=0; i<MAX_PLANTS; i++) {
        plants[i].t = plant_none;
        plants[i].month_left = -1;
    }
    plants[0].t = plant_ordinary;
    plants[1].t = plant_ordinary;

    if(cre) 
        SendMessage("# You are the Creator. "
                    "Wait for your partners to join, then type 'start'\n");
    if(is_spectator)
        SendMessage("# This game is already started. "
                    "You can only watch not play\n");
    SendMessage("# Type 'help' for game help.\n");
    SendPrompt();
}

ManagerGameSession::~ManagerGameSession()
{
    if(the_game) the_game->RemoveSession(this);
} 




#define MUST_BE_PLAYED \
        if(!the_game->IsStarted()) { \
            SendMessage("&- The game hasn't been started yet...\n");\
            return;\
        }\
        if(the_game->IsFinished()) { \
            SendMessage("&- The game is over, type quit to quit...\n");\
            return;\
        }

#define MUST_BE_TURN \
        if(is_turn_ended) { \
            SendMessage("&- You have already finished your turn...\n");\
            return;\
        }

#define MUST_BE_ACTIVE \
        if(is_spectator) { \
            SendMessage("&- You can only watch not play!\n");\
            return;\
        }

void ManagerGameSession::HandleCommand(const char *a_cmd) 
{
    ScriptVector cmd(a_cmd);
    if(cmd.Length()<1) {
        // empty line
	SendMessage("# Your name is ");
	SendMessage(GetName());
	SendMessage("\n");
        SendMessage("# You are in a gameroom. Enter a command. "
                    "Type 'help' to get help\n");
        SendMessage(the_game->GameStatusMessage());
        if(is_creator && the_game->GetNumplayers()>1) 
            SendMessage(
                "# Type 'start' to start the game, you're the Creator!\n");
        if(is_spectator && !the_game->IsFinished())
            SendMessage("# * You can only watch not play *\n");
    } else
    if(cmd[0] == "help") {
        if(is_creator) 
            SendMessage(
                "# start                 start the game!\n");
        SendMessage(
            "# market                get the current market status\n"
            "# info                  get your partners' info\n"
            "# buy <amount> <price>  place raw buy request\n"
            "# sell <amount> <price> place production sell request\n"
            "# prod <amount>         set production plan for this month\n"
            "# ?                     see your requests\n"
            "# build                 build a new plant\n"
            "# abuild                build a new automatic plant\n"
            "# upgrade               upgrade a plant to automatic\n"
            "# turn                  end turn\n"
            "# help                  for help\n"
            "# quit                  to resign and quit the game\n"
            "# chat on|off           switch global chat on or off\n"
            "# say <whattosay>       say something to your partners\n"
            "#                      (for global chat, use '.say')\n"
            "#\n"
            "#   Plant produces a prod unit for $2000 and a raw unit.\n"
            "#   Auto plant can do the same or produce 2 prod units\n"
            "#      for $3000 and 2 raw units.\n"
            "#   Plant can be build in 5 month for $2500+$2500.\n"
            "#   Automatic plant can be build in 7 month for $5000+$5000.\n"
            "#   Plant can be upgraded in 9 month for $3500+$3500 and\n"
            "#      all that period it continues to work as a plant.\n"
         "#   Monthly expenses are $300 per raw unit, $500 per prod. unit,\n"
         "#                $1000 per plant, $1500 per automatic plant.\n"
        );
    } else
    if(cmd[0] == "start") {
        if(is_creator) {
            if(the_game->GetNumplayers()>1) {
                the_game->Start();
                is_creator = false;    
            } else {
                SendMessage("&- This game cannot be played alone. "
                            "Wait for someone to join please\n");
            }
        } else {
            SendMessage("&- You are not the Creator "
                        "or the game is already started\n"); 
        }
    } else
    if(cmd[0] == "quit") {
         wishes_to_quit = true;
    } else
    if(cmd[0] == "buy") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         BuyArrange(cmd[1], cmd[2]);
    } else
    if(cmd[0] == "sell") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         SellArrange(cmd[1], cmd[2]);
    } else
    if(cmd[0] == "prod") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         ProdArrange(cmd[1]);
    } else
    if(cmd[0] == "build") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         BuildArrange(false);
    } else
    if(cmd[0] == "abuild") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         BuildArrange(true);
    } else
    if(cmd[0] == "upgrade") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         UpgradeArrange();
    } else
    if(cmd[0] == "turn") {
         MUST_BE_ACTIVE
         MUST_BE_PLAYED
         MUST_BE_TURN
         is_turn_ended = true;
         the_game->CheckEndTurn(); 
    } else
    if(cmd[0] == "market") {
         MUST_BE_PLAYED
         int raw, rawpr, prod, prodpr;
         the_game->GetMarket()->
              GetLevelParameters(the_game->GetAlivePlayers(), 
                                 raw, rawpr, prod, prodpr);
         ScriptVariable head(80, "%-10s %8s %9s  %8s %9s\n", 
                     "# ------", "Raw", "MinPrice", "Prod", "MaxPrice");
         ScriptVariable info(80, "%-10s %8d %9d  %8d %9d\n", 
                     "& MARKET", raw, rawpr, prod, prodpr);
         SendMessage(head.c_str());
         SendMessage(info.c_str());
         SendMessage("# ------ \n");
    } else
    if(cmd[0] == "info") {
         the_game->SendMeInfo(this);
    } else 
    if(cmd[0] == "?") {
         MUST_BE_PLAYED
         MUST_BE_ACTIVE
         ScriptVariable info(80, "# Requested: "
                                 "buy %d (for $%d per item) "
                                 "sell %d (for $%d per item) "
                                 "produce %d\n", 
                                 raw_request, raw_request_price,
                                 prod_request, prod_request_price,
                                 creation_request);
         SendMessage(info.c_str());
    } else 
    if(cmd[0] == "chat") {
        if(cmd[1] == "on") {
            chat_mode = chat_on;
            SendMessage("& OK chat is now on\n");
        } else 
        if(cmd[1] == "off") {
            chat_mode = chat_off;
            SendMessage("& OK chat is now off\n");
        } else 
        {
            SendMessage("&- use 'chat on' or 'chat off'\n");
        }
    } else 
    if(cmd[0] == "say") {
        ScriptVariable sv("# <");
        sv += GetName();
        sv += "> ";
        for(int i=1; i<cmd.Length(); i++) {
            sv += cmd[i];
            sv += " ";
        }
        sv += "\n";
        the_game->Broadcast(sv.c_str());
    } else 
    {
        SendMessage("&- Unknown command\n");
    }
    SendPrompt();
}
#undef MUST_BE_PLAYED
#undef MUST_BE_TURN
#undef MUST_BE_ACTIVE

void ManagerGameSession::TurnEnd()
{
    // finish the turn, close the bills

    // first, produce some produciton...
    int plant, aplant;
    GetPlants(plant, aplant); 
    int aprod = creation_request < 2 * aplant ? creation_request : 2 * aplant;
    int naprod = creation_request - aprod;
    if(aprod%2==1) { aprod--; naprod++; }
    int prodcost = (aprod/2)*3000 + naprod*2000;
    SendMessage(ScriptVariable(80, 
                "# You've created %d units at auto plants, "
                "%d at ordinary plants, it costs you $%d\n", 
                aprod, naprod, prodcost).c_str());
    money -= prodcost;
    raw -= creation_request;
    prod += creation_request;

    // then, check the build plants
    for(int i=0; i<MAX_PLANTS; i++) {
        switch(plants[i].t) {
            case plant_built:
                if((--plants[i].month_left)<1) {
                    SendMessage("# Plant construction finished!\n"
                                "& PLANT_BUILT\n");
                    plants[i].t = plant_ordinary;
                    money -= 2500;
                }
                break;
            case plant_abuilt:
                if((--plants[i].month_left)<1) {
                    SendMessage("# Automatic plant construction finished!\n"
                                "& AUTO_PLANT_BUILT\n");
                    plants[i].t = plant_automatic;
                    money -= 5000;
                }
                break;
            case plant_reconstructed:
                if((--plants[i].month_left)<1) {
                    SendMessage("# Plant upgrade finished!\n"
                                "& PLANT_UPGRADED\n");
                    plants[i].t = plant_automatic;
                    money -= 3500;
                }
                break;
            case plant_none:
            case plant_ordinary:
            case plant_automatic:
                // do nothing
                ;
        }
    }

    // now pay monthly expenses
    SendMessage(ScriptVariable(200, 
                "# You've payed $%d for storing %d raw units\n"
                "# You've payed $%d for storing %d production units\n"
                "# You've payed $%d for maintaining plants\n",
                300*raw, raw, 500*prod, prod, 1000*plant + 1500*aplant)
                .c_str());

    money -= (300*raw + 500*prod + 1000*plant + 1500*aplant);
    
    SendMessage(ScriptVariable(20, "# Your balance is $%d\n", money).c_str()); 

    if(money<0) {
        ScriptVariable sv(30, "& BANKRUPT %s\n", GetName());
        the_game->Broadcast(sv.c_str());
        SendMessage("# You are a bankrupt, sorry.\n");
        is_spectator = true;
    }
    // prepare for the next turn
    is_turn_ended = false;
    ClearRequests();
    SendMessage("& ENDTURN ------------------------------------------\n");
}

void ManagerGameSession::BuyRaw(int amount, int price)
{
    raw += amount;
    money -= amount*price;
    ScriptVariable msg(80, "& BOUGHT    %16s %10d %10d\n",
                           GetName(), amount, price);
    the_game->Broadcast(msg.c_str());
}

void ManagerGameSession::SellProd(int amount, int price)
{
    prod -= amount;
    money += amount*price;
    ScriptVariable msg(80, "& SOLD      %16s %10d %10d\n",
                           GetName(), amount, price);
    the_game->Broadcast(msg.c_str());
}

void ManagerGameSession::SendPrompt()
{
    /*SendMessage(". > ");*/
} 

const char* ManagerGameSession::GetStatus() const
{
#if 0
    return the_game->GetStatus();
#endif
    if(!the_game->IsStarted()) {
        status_string = "waiting ";
    } else
    if(the_game->IsFinished()) {
        status_string = "finished";
    } else
    if(IsSpectator()) {
        status_string = "watching";
    } else
        status_string = "playing ";
    status_string += ScriptVariable(0, " #%d", the_game->GetSeqnum());
    return status_string.c_str();
}

void ManagerGameSession::ClearRequests()
{
    raw_request = 0;
    raw_request_price = 0;
    prod_request = 0;
    prod_request_price = 0;
    creation_request = 0;
}

bool ManagerGameSession::ChatAccepted() const
{
    return (chat_mode == chat_on) || 
           (chat_mode == chat_notingame && 
               (!the_game->IsStarted() || is_spectator)
           );
}

void ManagerGameSession::
GetTradeRequests(int &r, int &rp, int &p, int &pp) const
{
    r  = raw_request;
    rp = raw_request_price;
    p  = prod_request;
    pp = prod_request_price;
}

void ManagerGameSession::GetPlants(int &ordinary, int &autopl) const
{
    ordinary = 0;
    autopl = 0;
    for(int i=0; i<MAX_PLANTS; i++) {
        switch(plants[i].t) {
            case plant_none: 
            case plant_built:
            case plant_abuilt:
                break;
            case plant_ordinary:
            case plant_reconstructed:
                ordinary++; break;
            case plant_automatic:
                autopl++; break;
        }
    }
}

void ManagerGameSession::
BuyArrange(const ScriptVariable &n, const ScriptVariable &pr)
{   
    long amount, price;
    if(!n.GetLong(amount) || !pr.GetLong(price)) {
        SendMessage("&- you must give two numbers (amount and price)\n");
        return;
    }
    int m_amount, m_price, m_skip;
    the_game->GetMarket()->GetLevelParameters(the_game->GetAlivePlayers(),
                               m_amount, m_price, m_skip, m_skip);
    if(m_amount<amount) {
        SendMessage("&- you want too much... the market doesn't have it\n");
        return;
    } 
    if(m_price>price) {
        SendMessage("&- your price is too low\n");
        return;
    } 
    raw_request = amount;
    raw_request_price = price;
    SendMessage("& OK   -- your request is accepted\n");
}

void ManagerGameSession::
SellArrange(const ScriptVariable &n, const ScriptVariable &pr)
{
    long amount, price;
    if(!n.GetLong(amount) || !pr.GetLong(price)) {
        SendMessage("&- you must give two numbers (amount and price)\n");
        return;
    }
    int m_amount, m_price, m_skip;
    the_game->GetMarket()->GetLevelParameters(the_game->GetAlivePlayers(),
                               m_skip, m_skip, m_amount, m_price);
    if(m_amount<amount) {
        SendMessage("&- you want too much... the market doesn't need it\n");
        return;
    } 
    if(amount>prod) {
        SendMessage("&- you don't have that many items to sell!\n");
        return;
    } 
    if(m_price<price) {
        SendMessage("&- your price is too high\n");
        return;
    } 
    prod_request = amount;
    prod_request_price = price;
    SendMessage("& OK   -- your request is accepted\n");
}

void ManagerGameSession::ProdArrange(const ScriptVariable &n)
{
    long amount;
    if(!n.GetLong(amount)) {
        SendMessage("&- you must give a number (amount)\n");
        return;
    }
    int p_ord, p_auto;
    GetPlants(p_ord, p_auto);
    if(amount > raw) {
        SendMessage("&- you don't have enough raw materials"
                    " to make so much production\n");
        return;
    }
    if(amount > p_ord + 2*p_auto) {
        SendMessage("&- you don't have enough plants"
                    " to make so much production\n");
        return;
    }
    creation_request = amount; 
    SendMessage("& OK   -- your request is accepted\n");
}

void ManagerGameSession::BuildArrange(bool autoplant)
{
    for(int i=0; i<MAX_PLANTS; i++) {
        if(plants[i].t == plant_none) {
            plants[i].t = autoplant ? plant_abuilt : plant_built;
            plants[i].month_left = autoplant ? 7 : 5;
            money -= autoplant ? 5000 : 2500;
            SendMessage("& OK   -- construction started!\n");
            return;
        }
    }
    SendMessage("&- How could you make so many plants?!\n");
}


void ManagerGameSession::UpgradeArrange()
{
    for(int i=0; i<MAX_PLANTS; i++) {
        if(plants[i].t == plant_ordinary) {
            plants[i].t = plant_reconstructed;
            plants[i].month_left = 9;
            money -= 3500;
            SendMessage("& OK   -- reconstruction started!\n");
            return;
        }
    }
    SendMessage("&- You've got no ordinary plant to upgrade\n");
}

