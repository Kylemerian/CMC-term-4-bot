#ifndef GAMEINFO_H
#define GAMEINFO_H

#include "structs.h"
#include "textline.h"

struct gameInfo
{
private:
    user * users;
    trading prices;
    int players;
    int alive;
    void initUsers(int servfd, textline & cmd);
    int findUser(user * users, textline & cmd);
    void cleanAliveFlag();
public:
    gameInfo();
    int getSell() const;
    int getBuy() const;
    void setPrices();
    void setUserInfo(int servfd, textline & cmd);
    void print() const;
    void setPrices(int servfd, textline & cmd);
    void setTradeResults(int servfd, textline & cmd);
    ~gameInfo();
};

#endif