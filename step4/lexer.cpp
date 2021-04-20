#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

const char * typeLex[] = {
    "number",
    "identificator",
    "keyword",
    "assign",
    "string",
    "operator",
    "separator"
};

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

int defType(int c)
{
    if(c == ' ' || c == '\t' || c == '\n' || c == ';' ||
        c == ',' || c == '}' || c == '{' || c == ')' || 
        c == '(' || c == ']' || c == '[')
        return sep;
    if(c >='0' && c <= '9')
        return digit;
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return alpha;
    if(c == '$' || c == '@' || c == '?')
        return special;
    if(c == '+' || c == '-' || c == '*' || c == '/' || 
        c == '%' || c == '<' || c == '>')
        return op;
    if(c == '=')
        return eq;
    if(c == ':')
        return dots;
    if(c == '\"')
        return quote;
    return -1;
}

int isSpace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

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
    void reverse();
public:
    //list * getLex();
    list * getLexList();
    void print();
    lexer();
    void sendChar(int c);
    ~lexer();
    int hasError();
};
/*
list * lexer::getLex()
{
    if(!initCur){
        reverse();
        curLex = lexems;
        initCur = !initCur;
    }
    list * tmp = curLex;
    if(curLex)
        curLex = curLex -> next;
    return tmp;
}
*/

list * lexer::getLexList()
{
    return lexems;
}

void lexer::print()
{
    reverse();
    list * tmp = lexems;
    if(status != E){
        while(tmp){
            printf("line %2d type = %13s   %s\n", tmp -> line, 
                typeLex[tmp -> type], tmp -> lex);
            tmp = tmp -> next;
        }
    }
}

void lexer::reverse()
{
    list * rev = NULL;
    for(list * qq = lexems, *next_node; qq != NULL; qq = next_node){
        next_node = qq->next;
        qq -> next = rev;
        rev = qq;
    }
    lexems = rev;
}

void lexer::freemem()
{
    while (lexems != NULL) {
        list * tmp = lexems;
        if (lexems -> lex != NULL)
            delete[] lexems -> lex;
        lexems = lexems -> next;
        delete tmp;
    }
}

list * lexer::addToList(char * key, int line, int type)
{
    list * tmp = new list;
    tmp -> line = line;
    tmp -> type = type;
    tmp -> lex = new char [strlen(key) + 1];
    strncpy(tmp -> lex, key, strlen(key));
    tmp -> lex[strlen(key)] = 0;
    tmp -> next = lexems;
    return tmp;
}

void lexer::number()
{
    if(type == op || type == eq || (type == sep && !isSpace(buf[size - 1]))){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, N);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, N);
        size = 0;
        status = H;
    }
    else if(type == digit)
        status = N;
    else
        error();
}

void lexer::ident()
{
    if(type == op || type == eq || type == dots || 
        (type == sep && !isSpace(buf[size - 1])))
    {
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, I);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, I);
        size = 0;
        status = H;
    }
    else if(type == alpha || type == digit)
        status = I;
    else
        error();
}

void lexer::keyword()
{
    if(type == op || type == eq || (type == sep && !isSpace(buf[size - 1]))){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, K);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, K);
        size = 0;
        status = H;
    }
    else if(type == alpha)
        status = K;
    else
        error();
    
}

void lexer::assign()
{
    if(type == eq){
        buf[size] = 0;
        lexems = addToList(buf, line, A);
        size = 0;
        status = H;
    }
    else
        error();
}

void lexer::str()
{
    if(type == quote){
        buf[size] = 0;
        lexems = addToList(buf, line, Str);
        size = 0;
        status = H;
    }
    else
        status = Str;
}

void lexer::start()
{
    if(size == 2 && (defType(buf[0]) == dots)){
        status = A;
        assign();
        return;
    }
    if(size == 2){
        buf[size] = buf[size - 1]; 
        buf[size - 1] = 0;
        if(defType(buf[0]) == op)
            lexems = addToList(buf, line, H);
        if(defType(buf[0]) == sep)
            lexems = addToList(buf, line, E);
        buf[0] = buf[size];
        size = 1;
    }
    if((type == op || type == eq) && size == 1){
        buf[1] = 0;
        lexems = addToList(buf, line, H);
        status = H;
        size = 0;
    }
    else if(type == quote)
        status = Str;
    else if(type == alpha)
        status = K;
    else if(type == digit)
        status = N;
    else if(type == special)
        status = I;
    else if(type == dots)
        status = A;
    else if(type == sep){
        if(!isSpace(buf[0])){
            buf[1] = 0;
            lexems = addToList(buf, line, E);
        }
        size = 0;
        status = H;
    }
    else
        error();
}

void lexer::switcher()
{
    type = defType(buf[size - 1]);
    if(status == N){
        number();
    }
    else if(status == I){
        ident();
    }
    else if(status == K){
        keyword();
    }
    else if(status == A){
        assign();
    }
    else if(status == Str){
        str();
    }
    else if(status == H){
        start();
    }
}

void lexer::error()
{
    printf("ERROR %s line %d status %d\n", buf, line, status);
    status = E;
    size = 0;
}

lexer::lexer()
{
    lexems = NULL;
    capacity = 256;
    buf = new char [capacity];
    size = 0;
    buf[0] = 0;
    line = 1;
    status = H;
}

void lexer::sendChar(int c)
{
    if(status == E)
        return;
    if(size == capacity)
        resize();
    type = defType(c);
    buf[size] = c;
    size++;
    //printf("char = %c line = %d\n", c, line);
    switcher();
    if(c == '\n')
        line++;
}

void lexer::resize()
{
    char * tmp = new char [capacity * 2];
    strncpy(tmp, buf, capacity);
    capacity *= 2;
    delete[] buf;
    buf = tmp;
}

lexer::~lexer()
{
    freemem();
    delete[] buf;
}

int lexer::hasError()
{
    return status == E;
}
/*
int main()
{
    int c;
    lexer obj;
    while((c = getchar()) != EOF)
        obj.sendChar(c);
    c = ' ';
    obj.sendChar(c);
    obj.print();
    return 0;
}
*/