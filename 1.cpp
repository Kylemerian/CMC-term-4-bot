#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

const char * botNick;


struct trading
{
    int sold;
    int bought;
    int numSold;
    int numBought;
};

struct user
{
    int raw;
    int prod;
    int factories;
    int money;
    //int isCreator;
    int isAlive;
    char * nick;
    trading results;
    user()
    {
        results.sold = 0;
        results.numSold = 0;
        results.bought = 0;
        results.numBought = 0;
        isAlive = 1;
    }
};

struct textline
{
private:
    int capacity;
    char * buf;
    int size;
    int gmEnd;
    int flagRead;
    void resize();
public:    
    textline()
    {
        capacity = 256;
        buf = (char*) malloc(capacity);
        size = 0;
        buf[0] = 0;
        gmEnd = 0;
        flagRead = 1;
    };
    int hasSubStr(const char * str) const
    {
        return (strstr(buf, str) != NULL);
    }
    int isEnded() const
    {
        return gmEnd;
    }
    void print()
    {
        printf("%s\n", buf);
    }
    ~textline()
    {
        free(buf);
    }
    void getNext(int servfd);
    void setPrices(trading * trd);
    void cleanAllChars();
    void getFirstNum(int & a)
    {
        sscanf(buf, "%d", &a);
    }
    void getTwoNum(int & a, int &b)
    {
        sscanf(buf, "%d%d", &a, &b);
    }
    void setRsrcs(int servfd, user * usr);
    char * getNick();
};

int isDigit(int c)
{
    return(c >= '0' && c <= '9');
}

int isAlpha(int c)
{
    return((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

void textline::cleanAllChars()
{
    int i = 1;
    while(buf[i] != 0){
        if(isAlpha(buf[i - 1]) || isAlpha(buf[i + 1]))
            buf[i] = ' ';
        i++;
    }
    i = 0;
    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
}

char * textline::getNick()
{
    char * res;
    char * tmp = strstr(buf, "INFO") + 5;
    int i = 0;
    while(tmp[i] == ' ' || tmp[i] == '\t')
        i++;
    int lrange = i;
    while(tmp[i] != ' ' && tmp[i] != '\t')
        i++;
    int rrange = i;
    res = (char *)malloc(rrange - lrange + 1);
    strncpy(res, &tmp[lrange], rrange - lrange);
    res[rrange - lrange] = 0;
    return res;
}

void textline::resize()
{
    char * tmp = (char *)malloc(2 * capacity);
    strncpy(tmp, buf, size);
    free(buf);
    buf = tmp;
    capacity *= 2;
}

void textline::setRsrcs(int servfd, user * usr)
{
    cleanAllChars();
    sscanf(buf, "%d%d%d%d", &usr->raw, &usr->prod, 
        &usr->money, &usr->factories);
    usr->isAlive = 1;
}

void textline::setPrices(trading * trd)
{
    int i = 0;
    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
    sscanf(buf, "%d%d%d%d", &i, &trd->bought, &i, &trd->sold);
}

void textline::getNext(int servfd)
{
    buf[size] = 0;
    size = 0;
    while(1){
        flagRead = read(servfd, &buf[size], 1);
        if(buf[size] == '\n' || flagRead == -1)
            break;
        if(buf[size] == '\r'){
            buf[size] = ' ';
        }
        size++;
        if(size == capacity)
            this -> resize();
    }
    buf[size] = 0;
    if(hasSubStr("WIN")){
        if(!hasSubStr("YOU"))
            printf("DEFEAT\n");
        else
            printf("VICTORY\n");
        gmEnd = 1;
    }
    //printf("DBG %s\n", buf);
}

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
    gameInfo()
    {
        users = NULL;
        alive = -1;
    }
    int getSell() const
    {
        return prices.sold;
    }
    int getBuy() const
    {
        return prices.bought;
    }
    void setPrices();
    void setUserInfo(int servfd, textline & cmd);
    void print() const;
    void setPrices(int servfd, textline & cmd);
    void setTradeResults(int servfd, textline & cmd);
    ~gameInfo();
};

gameInfo::~gameInfo()
{
    for (int i = 0; i < players; i++)
        free(users[i].nick);
    free(users);
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
            printf("%10s %3d %4d %6d %6d %4d %7d %3d %4d\n", users[i].nick, users[i].raw,
                users[i].prod, users[i].money, users[i].factories, users[i].results.numBought,
                    users[i].results.bought, users[i].results.numSold, users[i].results.sold);
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

void createOrJoin(int argc, char ** argv, int servfd, textline & cmd)
{
    if(argc == 5){
        dprintf(servfd, ".join %s\n", argv[4]);
    }
    else{
        int pcnt = 0;
        dprintf(servfd, ".create\n");
        while(pcnt != atoi(argv[5])){
            cmd.getNext(servfd);
            if(cmd.hasSubStr("JOIN"))
                pcnt++;
            if(cmd.hasSubStr("LEFT"))
                pcnt--;
        }
        dprintf(servfd, "start\n");
    }             
    do
    {
        cmd.getNext(servfd);
    } while(!cmd.hasSubStr("START"));
}

void prepare4Game(int servfd, char ** args, int argc)
{
    textline cmd;
    gameInfo game;
    botNick = args[3];
    dprintf(servfd, "%s\n", args[3]);
    createOrJoin(argc, args, servfd, cmd);
    while(!cmd.isEnded()){
        game.setUserInfo(servfd, cmd);
        game.setPrices(servfd, cmd);
        
        dprintf(servfd,"buy 2 %d\nsell 2 %d\n", game.getBuy(), game.getSell());
        dprintf(servfd, "prod 2\nturn\n");

        game.setTradeResults(servfd, cmd);
        if(!cmd.isEnded())
            game.print();
    }
}

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in addr;
    if(argc != 5 && argc != 6){ 
        //to be changed (now ip+port+nick/room to join + botNick)
        printf("Wrong number of args\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    if(!inet_aton(argv[1], &(addr.sin_addr))){
        printf("IP-convertation error\n");
        exit(1);
    }
    if(0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))){
        printf("Connection error\n");
        exit(1);
    }
    prepare4Game(sockfd, argv, argc);
    return 0;
}
