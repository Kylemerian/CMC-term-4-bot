#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

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