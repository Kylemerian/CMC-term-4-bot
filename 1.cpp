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
    char * name;
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
        capacity = 64;
        buf = new char(capacity);
        size = 0;
        buf[size] = 0;
    };
    /*int isBotLine() const
    {
        return (buf[0] == '%');
    }*/
    void getNext(int servfd);
    void print(){
        printf("%s\n", buf);
    }
};

void textline::getNext(int servfd)
{
    int c;
    size = 0;
    int res;
    do {
        res = read(servfd, &c, 1);
    } while (res == 1 && c != '\n');
    while(c != '\n' && !res){
        res = read(1, &buf[size], 1);
        size++;
    };
    buf[size] = 0;
}

void prepare4Game(int servfd)
{
    textline cmd;
    dprintf(servfd, "DiN\n");
    cmd.getNext(servfd);
    cmd.print();
    cmd.getNext(servfd);
    cmd.print();
}

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in addr;
    if(argc != 3){  //to be changed (now ip+port)
        printf("Wrong number of args\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    if(!inet_aton(argv[1], &(addr.sin_addr))){
        /*error*/
    }
    if(0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))){
        /*error*/
    }
    prepare4Game(sockfd);
    return 0;
}