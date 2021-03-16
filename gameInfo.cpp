#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "textline.h"
#include "structs.h"

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

gameInfo::gameInfo()
{
    users = NULL;
    alive = -1;
}

gameInfo::~gameInfo()
{
    for (int i = 0; i < players; i++)
        free(users[i].nick);
    free(users);
}

int gameInfo::getSell() const
{
    return prices.sold;
}

int gameInfo::getBuy() const
{
    return prices.bought;
}

void gameInfo::setPrices(int servfd, textline & cmd)
{
    dprintf(servfd, "market\n");
    do
    {
        cmd.getNext(servfd);
    } while(!cmd.hasSubStr("MARKET") && !cmd.isEnded());
    cmd.setPrices(&prices);
    cmd.getNext(servfd);
}

void gameInfo::print() const
{
    printf("---------------INFO-------------------\n");
    printf("nick       raw prod  money plants  num  Bought num Sold\n");
    for (int i = 0; i < players; i++){
        if(users[i].isAlive){
            printf("%10s %3d %4d %6d %6d %4d %7d %3d %4d\n", users[i].nick, 
                users[i].raw, users[i].prod, users[i].money,users[i].factories,
                users[i].results.numBought,users[i].results.bought,
                users[i].results.numSold, users[i].results.sold);
        }
    }
    printf("BUY PRICE = %d\nSELL PRICE = %d\n", prices.bought, prices.sold);
}

void gameInfo::cleanAliveFlag()
{
    for (int i = 0; i < players; i++){
        users[i].isAlive = 0;
    }
}

int gameInfo::findUser(user * users, textline & cmd)
{
    for(int i = 0; i < players; i++){
        if(cmd.hasSubStr(users[i].nick))
            return i;
    }
    return -1;
}

void gameInfo::initUsers(int servfd, textline & cmd)
{
    users = (user *)malloc(alive * sizeof(user));
    players = alive;
    dprintf(servfd, "info\n");
    for (int i = 0; i < alive; i++){
        do{
            cmd.getNext(servfd);
        } while(!cmd.hasSubStr("&"));
        users[i].nick = cmd.getNick();
    }
    do{
        cmd.getNext(servfd);
    } while(!cmd.hasSubStr("PLAYERS"));
}

void gameInfo::setUserInfo(int servfd, textline & cmd)
{
    int tmp;
    dprintf(servfd, "info\n");
    while(!cmd.hasSubStr("PLAYERS"))
        cmd.getNext(servfd);
    cmd.cleanAllChars();
    cmd.getFirstNum(tmp);
    alive = tmp;
    //printf("ALIVE NOW = %d\n", alive);
    if(users == NULL)
        initUsers(servfd, cmd);
    cleanAliveFlag();
    dprintf(servfd, "info\n");
    for(int i = 0; i < alive; i++){
        do{
            cmd.getNext(servfd);
        } while(!cmd.hasSubStr("&"));
        tmp = findUser(users, cmd);
        if(tmp != -1)
            cmd.setRsrcs(servfd, &(users[tmp]));
    }
}

void gameInfo::setTradeResults(int servfd, textline & cmd)
{
    for (int i = 0; i < players; i++){
        users[i].results.bought = 0;
        users[i].results.numBought = 0;
        users[i].results.sold = 0;
        users[i].results.numSold = 0;
    }
    
    while(!cmd.hasSubStr("ENDTURN")){
        cmd.getNext(servfd);
        if(cmd.isEnded())
            break;
        if(cmd.hasSubStr("BOUGHT")){
            int tmp;
            for (int i = 0; i < players; i++)
                if(cmd.hasSubStr(users[i].nick))
                    tmp = i;
            cmd.cleanAllChars();
            cmd.getTwoNum(users[tmp].results.numBought, users[tmp].results.bought);
        }
        if(cmd.hasSubStr("SOLD")){
            int tmp;
            for (int i = 0; i < players; i++)
                if(cmd.hasSubStr(users[i].nick))
                    tmp = i;
            cmd.cleanAllChars();
            cmd.getTwoNum(users[tmp].results.numSold, users[tmp].results.sold);
        }
    }
}