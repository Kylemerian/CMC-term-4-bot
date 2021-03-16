#ifndef STRUCTS_H
#define STRUCTS_H

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
    user();
};

#endif