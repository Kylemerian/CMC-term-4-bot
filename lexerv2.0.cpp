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
    "operator"
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
    list * lexems;
    void number();
    void ident();
    void keyword();
    void assign();
    void str();
    void start();
    void error();
    void resize();
    list * addToList(char * key, int line, int type);
    void printList(list * head);
    void freemem();
public:
    void print()
    {
        list * tmp = lexems;
        printList(tmp);
    };
    lexer();
    void setChar(int ac);
    void switcher();
    ~lexer();
};

void lexer::freemem()
{
    if (lexems != NULL) {
        if (lexems -> lex != NULL)
            delete[] lexems -> lex;
        list * tmp = lexems;
        lexems = lexems -> next;
        delete tmp;
        freemem();
    }
}

void lexer::printList(list * head)
{
    if (head != NULL) {
        printList(head -> next);
        printf("line %2d type = %13s   %s\n", head -> line, 
            typeLex[head -> type], head -> lex);
    }
}

list * lexer::addToList(char * key, int line, int type)
{
    list * tmp = new list;
    tmp -> line = line;
    tmp -> type = type;
    tmp -> lex = new char (strlen(key) + 1);
    strncpy(tmp -> lex, key, strlen(key));
    tmp -> lex[strlen(key)] = 0;
    tmp -> next = lexems;
    return tmp;
}

void lexer::number()
{
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, N);
        //printf("line = %d  %s\n", line, buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, N);
        //printf("line = %d  %s\n", line, buf);
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
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, I);
        //printf("line = %d  %s\n", line, buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, I);
        //printf("line = %d  %s\n", line, buf);
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
    if(type == op || type == eq){
        buf[size] = buf[size - 1];
        buf[size - 1] = 0;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, K);
        //printf("line = %d  %s\n", line, buf);
        //printf("%s\n", &buf[size]);
        buf[0] = buf[size];
        size = 1;
        status = H;
    }
    else if(type == sep){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, K);
        //printf("line = %d  %s\n", line, buf);
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
        //printf("line = %d  %s\n", line, buf);
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
        //printf("line = %d  %s\n", line, buf);
        size = 0;
        status = H;
    }
    else
        status = Str;
}

void lexer::start()
{
    if(size == 2){
        buf[size] = buf[size - 1]; 
        buf[size - 1] = 0;
        lexems = addToList(buf, line, H);
        //printf("line = %d  %s\n", line, buf);
        buf[0] = buf[size];
        size = 1;
    }
    if((type == op || type == eq) && size == 1){
        buf[1] = 0;
        lexems = addToList(buf, line, H);
        //printf("line = %d  %s\n", line, buf);
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
    status = E;
}

lexer::lexer()
{
    lexems = NULL;
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
    (c == '\n') ? line++: 0;
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
    obj.print();
    return 0;
}