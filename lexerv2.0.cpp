#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

enum{
    N,
    I,
    K,
    A,
    Str,
    E,
    H
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

int defType(int c)
{
    if(c == ' ' || c == '\t' || c == '\n' || c == ';' || c == ',')
        return sep;
    if(c >='0' && c <= '9')
        return digit;
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return alpha;
    if(c == '$' || c == '@' || c == '?')
        return special;
    if(c == '+' || c == '-' || c == '*' || c == '/' || 
        c == '%' || c == '<' || c == '>'|| c == ')' || 
        c == '(' || c == ']' || c == '[')
        return op;
    if(c == '=')
        return eq;
    if(c == ':')
        return dots;
    if(c == '\"')
        return quote;
    return -1;
}

class lexer{
    char * buf;
    int size;
    int type;
    int capacity;
    int line;
    int status;
    void number();
    void ident();
    void keyword();
    void assign();
    void str();
    void start();
    void error();
    void resize();
public:
    lexer();
    void setChar(int ac);
    void switcher();
    ~lexer();
};

void lexer::number()
{
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        printf("%s\n", buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        printf("%s\n", buf);
        size = 0;
        status = H;
    }
    else if(type == alpha)
        status = N;
}

void lexer::ident()
{
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        printf("%s\n", buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        printf("%s\n", buf);
        size = 0;
        status = H;
    }
    else if(type == alpha || type == digit)
        status = I;
}

void lexer::keyword()
{
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        printf("%s\n", buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        printf("%s\n", buf);
        size = 0;
        status = H;
    }
    else if(type == alpha)
        status = K;
    
}

void lexer::assign()
{
    if(type == eq){
        buf[size] = 0;
        printf("%s\n", buf);
        size = 0;
        status = H;
    }
}

void lexer::str()
{
    if(type == quote){
        buf[size] = 0;
        printf("%s\n", buf);
        size = 0;
        status = H;
    }
    else
        status = Str;
}

void lexer::start()
{
    if(size == 1 && (defType(buf[0]) != sep) && defType(buf[0]) != special){
        buf[size] = 0;
        printf("%s\n", buf);
        size = 0;
    }
    if(type == op || type == eq)
        status = H;
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
        size = 0;
        status = H;
    }
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
    status = E;
}

lexer::lexer()
{
    capacity = 256;
    buf = new char (capacity);
    size = 0;
    buf[0] = 0;
    line = 1;
    status = H;
}

void lexer::setChar(int c)
{
    if(size == capacity)
        resize();
    type = defType(c);
    buf[size] = c;
    size++;
}

void lexer::resize()
{
    char * tmp = new char (capacity * 2);
    strncpy(tmp, buf, capacity);
    capacity *= 2;
    delete[] buf;
    buf = tmp;
}

lexer::~lexer()
{
    delete[] buf;
}

int main()
{
    int c;
    lexer obj;
    while((c = getchar()) != EOF){
        obj.setChar(c);
        obj.switcher();
    }
    c = ' ';
    obj.setChar(c);
    obj.switcher();
    return 0;
}