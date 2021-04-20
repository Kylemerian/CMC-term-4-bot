#ifndef LEXER_H
#define LEXER_H

enum{
    N,
    I,
    K,
    A,
    Str,
    H,
    E
};

enum{
    op,
    sep,
    digit,
    alpha,
    dots,
    eq,
    special,
    quote
};

struct list
{
    int line;
    char * lex;
    int type;
    list * next;
};

int defType(int c);

int isSpace(int c);

class lexer{
    char * buf;
    int size;
    int type;
    int capacity;
    int line;
    int status;
    //int initCur;
    //list * curLex;
    list * lexems;
    void switcher();
    void number();
    void ident();
    void keyword();
    void assign();
    void str();
    void start();
    void error();
    void resize();
    list * addToList(char * key, int line, int type);
    void freemem();
public:
    //list * getLex();
    void reverse();
    int hasError();
    void print();
    list * getLexList();
    lexer();
    void sendChar(int c);
    ~lexer();
};

#endif