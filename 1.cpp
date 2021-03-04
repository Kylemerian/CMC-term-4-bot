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
};

struct auction
{
    int sold;
    int bought;
};

struct textline
{
private:
    int capacity;
    char * buf;
public:    
    int size;
    textline()
    {
        capacity = 256;
        buf = (char*) malloc(capacity);
        size = 0;
        buf[0] = 0;
        buf[255] = 0;/**/
    };
    int isBotLine() const
    {
        return (buf[0] == '&');
    }
    void getNext(int servfd);
    void print() const
    {
        printf("%s\n", buf);
    }
    int hasSubStr(const char * str) const
    {
        return (strstr(buf, str) != nullptr);
    }
    ~textline()
    {
        free(buf);
    }
    //resize
};

void textline::getNext(int servfd)
{
    buf[size] = 0;
    size = 0;

    while((size + 1 < capacity) ){
        read(servfd, &buf[size], 1);
        if(buf[size] == '\n')
            break;
        if(buf[size] == '\r'){
            buf[size] = ' ';
        }
        //printf("%c", buf[size]);
        size++;
    }
    /*printf("    size = %d\n", size);*/
    buf[size] = 0;
    /**/printf("DBG %s\n", buf);
    
}

void prepare4Game(int servfd, char ** nicks)
{
    textline cmd;
    dprintf(servfd, "%s\n", nicks[4]);
    dprintf(servfd, ".join %s\n", nicks[3]);
    cmd.getNext(servfd);
    while(!cmd.hasSubStr("START"))
        cmd.getNext(servfd);
    while(1){
        dprintf(servfd, "buy 2 600\nsell 2 4500\nproduce 2\nturn\n");
        cmd.getNext(servfd);
        while(!cmd.isBotLine()) /**/
            cmd.getNext(servfd);
        if(cmd.hasSubStr("WIN"))
            break;
        while(!cmd.isBotLine()) /**/
            cmd.getNext(servfd);
        if(cmd.hasSubStr("WIN"))
            break;
        while(!cmd.isBotLine()) /**/
            cmd.getNext(servfd);
        if(cmd.hasSubStr("WIN"))
            break;
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