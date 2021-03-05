#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct user
{
    int raw;
    int prod;
    int factories;
    int money;
    int isBankrupt;
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
    void resize();
public:    
    textline()
    {
        capacity = 256;
        buf = (char*) malloc(capacity);
        size = 0;
        buf[0] = 0;
        gmEnd = 0;
    };
    /*void print() const
    {
        printf("%s\n", buf);
    }*/
    int hasSubStr(const char * str) const
    {
        return (strstr(buf, str) != nullptr);
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
};

int isDigit(int c)
{
    return(c >= '0' && c <= '9');
}

void textline::resize()
{
    char * tmp = (char *)malloc(2 * capacity);
    strncpy(tmp, this -> buf, this -> size);
    free(this -> buf);
    this -> buf = tmp;
    capacity *= 2;
}

void textline::setResrcs(int servfd, user * bot)
{
    int i = 0;
    dprintf(servfd, "info\n");
    while(!this->hasSubStr(bot->nick))
        this->getNext(servfd);

    while(this -> buf[i] != 0){
        if(!isDigit(this -> buf[i]))
            this -> buf[i] = ' ';
        i++;
    }
    sscanf(this -> buf, "%d%d%d%d", &bot->raw, &bot->prod, 
        &bot->money, &bot->factories);
}

void textline::setPrices(int servfd, trading * trd)
{
    int i = 0;
    dprintf(servfd, "market\n");
    this->getNext(servfd);
    this->getNext(servfd);

    while(this -> buf[i] != 0){
        if(!isDigit(this -> buf[i]))
            this -> buf[i] = ' ';
        i++;
    }
    sscanf(this -> buf, "%d%d%d%d", &i, &trd->bought, &i, &trd->sold);
    printf("%d %d\n", trd->bought, trd->sold);
    this->getNext(servfd);
}

void textline::getNext(int servfd)
{
    buf[size] = 0;
    size = 0;

    while(1){
        read(servfd, &buf[size], 1);
        if(buf[size] == '\n')
            break;
        if(buf[size] == '\r'){
            buf[size] = ' ';
        }
        size++;
        if(size == capacity)
            this -> resize();
    }
    buf[size] = 0;
    if(this -> hasSubStr("WIN")){
        /**/printf("        win was here\n");
        this -> gmEnd = 1;
    }
    printf("DBG %s\n", buf);
    
}

void prepare4Game(int servfd, char ** nicks)
{
    textline cmd;
    user bot;
    trading curTrade;
    bot.nick = nicks[4];
    dprintf(servfd, "%s\n", nicks[4]);
    dprintf(servfd, ".join %s\n", nicks[3]);
    cmd.getNext(servfd);
    while(!cmd.hasSubStr("START"))
        cmd.getNext(servfd);
    while(!cmd.isEnded()){
        cmd.setPrices(servfd, &curTrade);
        cmd.setResrcs(servfd, &bot);

        dprintf(servfd,"buy 2 %d\nsell 2 %d\n",curTrade.bought,curTrade.sold);
        dprintf(servfd, "prod 2\nturn\n");

        while(!cmd.hasSubStr("ENDTURN")){
            /**/if(cmd.isEnded())
            /**/    break;
            cmd.getNext(servfd);
        }
    }
}

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in addr;
    if(argc != 5){  //to be changed (now ip+port+nick/room to join + botNick)
        printf("Wrong number of args\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    if(!inet_aton(argv[1], &(addr.sin_addr))){
        printf("IP-convertation error\n");
    }
    if(0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))){
        printf("Connection error\n");
    }
    prepare4Game(sockfd, argv);
    return 0;
}