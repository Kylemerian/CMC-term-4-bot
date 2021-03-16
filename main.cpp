#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "gameInfo.h"
#include "textline.h"
#include "structs.h"

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
    //botNick = args[3];
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
        perror("Wrong number of args\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    if(!inet_aton(argv[1], &(addr.sin_addr))){
        perror("IP-convertation error\n");
        exit(1);
    }
    if(0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))){
        perror("Connection error\n");
        exit(1);
    }
    prepare4Game(sockfd, argv, argc);
    return 0;
}
