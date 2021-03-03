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
        buf = new char(capacity);
        size = 0;
        buf[size] = 0;
    };
    int isBotLine() const
    {
        return (buf[0] == '%');
    }
    void getNext(int servfd);
    void print(){
        printf("%s\n", buf);
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
    //printf("%s\n", buf);
    
}

void prepare4Game(int servfd, char * idRoom)
{
    textline cmd;
    dprintf(servfd, "DiN\n");
    dprintf(servfd, ".join %s\n", idRoom);
    while(1){
        cmd.getNext(servfd);        //skipping .help
        if(cmd.isBotLine())
            cmd.print();
        cmd.getNext(servfd);        //game started
        
    }
}

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in addr;
    if(argc != 4){  //to be changed (now ip+port+nick/room to join)
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
    prepare4Game(sockfd, argv[3]);
    return 0;
}