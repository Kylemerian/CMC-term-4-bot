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
    int isCreator;
    const char * nick;
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
    int isEnded()
    {
        return gmEnd;
    }
    ~textline()
    {
        free(buf);
    }
    void getNext(int servfd);
    void setPrices(int servfd, trading * trd);
    void setResrcs(int servfd, user * bot);
    void cleanReqs(int servfd);
};

int isDigit(int c)
{
    return(c >= '0' && c <= '9');
}

void textline::resize()
{
    char * tmp = (char *)malloc(2 * capacity);
    strncpy(tmp, buf, size);
    free(buf);
    buf = tmp;
    capacity *= 2;
}

void textline::setResrcs(int servfd, user * bot)
{
    int i = 0;
    dprintf(servfd, "info\n");
    while(!hasSubStr(bot->nick))
        getNext(servfd);

    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
    sscanf(buf, "%d%d%d%d", &bot->raw, &bot->prod, 
        &bot->money, &bot->factories);
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
    if(hasSubStr("WIN") || hasSubStr("BANKRUPT"))
        gmEnd = 1;
    printf("DBG %s\n", buf);
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

void printGameInfo(user bot, trading trade)
{
    printf("@@ price prod = %d\n", trade.sold);
    printf("@@ price raw = %d\n", trade.bought);

    printf("@@ money = %d\n", bot.money);
    printf("@@ raw = %d\n", bot.raw);
    printf("@@ prod = %d\n", bot.prod);
    printf("@@ plants = %d\n", bot.factories);
}

void prepare4Game(int servfd, char ** args, int argc)
{
    textline cmd;
    user bot;
    trading curTrade;
    bot.nick = args[3];
    dprintf(servfd, "%s\n", args[3]);
    createOrJoin(argc, args, servfd, cmd);
    while(!cmd.isEnded()){
        cmd.setPrices(servfd, &curTrade);
        cmd.setResrcs(servfd, &bot);
        
        dprintf(servfd,"buy 2 %d\nsell 2 %d\n",curTrade.bought,curTrade.sold);
        dprintf(servfd, "prod 2\nturn\n");
        
        printGameInfo(bot, curTrade);
        
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
