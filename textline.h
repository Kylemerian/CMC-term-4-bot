#ifndef TEXTLINE_H
#define TEXTLINE_H

#include "structs.h"

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
    textline();
    int hasSubStr(const char * str) const;
    int isEnded() const;
    void print() const;
    ~textline();
    void getNext(int servfd);
    void setPrices(trading * trd);
    void cleanAllChars();
    void getFirstNum(int & a) const;
    void getTwoNum(int & a, int &b) const;
    void setRsrcs(int servfd, user * usr);
    char * getNick();
};

int isDigit(int c);

int isAlpha(int c);

#endif