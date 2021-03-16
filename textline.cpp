#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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
    void getNext(int servfd);
    void setPrices(trading * trd);
    void cleanAllChars();
    void getFirstNum(int & a) const;
    void getTwoNum(int & a, int &b) const;
    void setRsrcs(int servfd, user * usr);
    char * getNick();
    ~textline();
};

int isDigit(int c)
{
    return(c >= '0' && c <= '9');
}

int isAlpha(int c)
{
    return((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

textline::textline()
{
    capacity = 256;
    buf = (char*) malloc(capacity);
    size = 0;
    buf[0] = 0;
    gmEnd = 0;
    flagRead = 1;
};

int textline::hasSubStr(const char * str) const
{
    return (strstr(buf, str) != NULL);
}

int textline::isEnded() const
{
    return gmEnd;
}

void textline::print() const
{
    printf("%s\n", buf);
}

void textline::cleanAllChars()
{
    int i = 1;
    while(buf[i] != 0){
        if(isAlpha(buf[i - 1]) || isAlpha(buf[i + 1]))
            buf[i] = ' ';
        i++;
    }
    i = 0;
    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
}

void textline::getFirstNum(int & a) const
{
    sscanf(buf, "%d", &a);
}

void textline::getTwoNum(int & a, int &b) const
{
    sscanf(buf, "%d%d", &a, &b);
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
    sscanf(buf, "%d%d%d%d", &usr->raw, &usr->prod, 
        &usr->money, &usr->factories);
    usr->isAlive = 1;
}

void textline::setPrices(trading * trd)
{
    int i = 0;
    while(buf[i] != 0){
        if(!isDigit(buf[i]))
            buf[i] = ' ';
        i++;
    }
    sscanf(buf, "%d%d%d%d", &i, &trd->bought, &i, &trd->sold);
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
    if(hasSubStr("WIN")){
        if(!hasSubStr("YOU"))
            printf("DEFEAT\n");
        else
            printf("VICTORY\n");
        gmEnd = 1;
    }
    //printf("DBG %s\n", buf);
}

textline::~textline()
{
    free(buf);
}