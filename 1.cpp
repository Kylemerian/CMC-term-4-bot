#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct user
{
    int raw;
    int prod;
    int factories;
    int money;
    //int isCreator;
    int isAlive;
    char * nick;
};

struct trading
{
    int sold;
    int bought;
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
    void setPrices(int servfd, trading * trd);
    void cleanAllChars();
    void getFirstNum(int & a)
    {
        sscanf(buf, "%d", &a);
    }
    void setRsrcs(int servfd, user * usr);
    char * getNick();
    //void setResrcs(int servfd, user * bot);
};

int isDigit(int c)
{
    return(c >= '0' && c <= '9');
}

void textline::cleanAllChars()
{
    int i = 0;
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
    printf("NICK = %s\n", res);
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
    //print();
    sscanf(buf, "%d%d%d%d", &usr->raw, &usr->prod, 
        &usr->money, &usr->factories);
    /*printf("  %s %d %d %d %d\n", usr->nick, usr->raw, usr->prod, 
        usr->money, usr->factories);*/
    usr->isAlive = 1;
}

void textline::setPrices(int servfd, trading * trd)
{
    int i = 0;
    dprintf(servfd, "market\n");
    do
    {
        getNext(servfd);
    } while(!hasSubStr("MARKET"));
    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
    sscanf(buf, "%d%d%d%d", &i, &trd->bought, &i, &trd->sold);
    printf("%d %d\n", trd->bought, trd->sold);
    getNext(servfd);
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
    if(hasSubStr("WIN")) //to add bot nick
        gmEnd = 1;
    //printf("DBG %s\n", buf);
}

struct gameInfo
{
private:
    user * users;
    trading prices;
    //trade;
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
    void setPrices();
    void setUserInfo(int servfd, textline & cmd);
    void print();
};

void gameInfo::print()
{
    printf("--------------INFO------------\n");
    printf("nick raw prod money plants\n");
    for (int i = 0; i < alive; i++){
        if(users[i].isAlive)
            printf("%s %d %d %d %d\n", users[i].nick, users[i].raw,
                users[i].prod, users[i].money, users[i].factories);
    }
    
    
}

void gameInfo::cleanAliveFlag()
{
    for (int i = 0; i < alive; i++){
        users[i].isAlive = 0;
    }
}

int gameInfo::findUser(user * users, textline & cmd)
{
    for(int i = 0; i < alive; i++){
        if(cmd.hasSubStr(users[i].nick))
            return i;
    }
}

void gameInfo::initUsers(int servfd, textline & cmd)
{
    users = (user *)malloc(alive * sizeof(user));
    dprintf(servfd, "info\n");
    for (int i = 0; i < alive; i++){
        do{
            cmd.getNext(servfd);
        } while(!cmd.hasSubStr("&"));
        /*printf("line for nick\n");*/
        /*cmd.print();*/
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
    if(users == NULL)
        initUsers(servfd, cmd);
    cleanAliveFlag();
    dprintf(servfd, "info\n");
    for(int i = 0; i < alive; i++){
        do{
            cmd.getNext(servfd);
        } while(!cmd.hasSubStr("&"));
        tmp = findUser(users, cmd);
        cmd.setRsrcs(servfd, &(users[tmp]));
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
    //bot.nick = args[3];
    dprintf(servfd, "%s\n", args[3]);
    createOrJoin(argc, args, servfd, cmd);
    while(!cmd.isEnded()){
        game.setUserInfo(servfd, cmd);
        game.print();
        //cmd.setPrices(servfd, &curTrade);
        //cmd.setResrcs(servfd, &bot);
        
        dprintf(servfd,"buy 600 \nsell 2 4000\n");
        dprintf(servfd, "prod 2\nturn\n");
        
        //printGameInfo(bot, curTrade);
        
        while(!cmd.hasSubStr("ENDTURN")){
            cmd.getNext(servfd);
            if(cmd.isEnded())
                //printf("        I was here\n");
                break;
        }
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
