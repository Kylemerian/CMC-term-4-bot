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
    "quote",
    "operator",
    "index",
    "arg",
    "callbracket"
};

enum typesS
{
    sep,
    digit,
    alpha,
    special,
    op,
    eq,
    dots,
    quotes,
    idxl,
    idxr,
    funcl,
    funcr
};

enum typesL
{
    N,
    I,
    K,
    A,
    S,
    Q,
    IDX,
    ARG,
    CLL
};

struct list
{
    int line;
    char * lex;
    int type;
    list * next;
};

int defineType(int c)
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
        c == '%' || c == '<' || c == '>')
        return op;
    if(c == '=')
        return eq;
    if(c == ':')
        return dots;
    if(c == '\"')
        return quotes;
    if(c == '[')
        return idxl;
    if(c == ']')
        return idxr;
    if(c == ')')
        return funcr;
    if(c == '(')
        return funcl;
    return -1;
}

class lexer
{
    char * buf;
    int size;
    int type;
    int c;
    int capacity;
    int line;
    list * lexems;
    void freemem();
    void getChar();
    void number();
    void ident();
    void keyword();
    void quote();
    void assign();
    void indNum();
    void index();
    void resize();
    void error();
    void funccall();
    void funcArg();
    list * addToList(char * key, int line, int type);
    void printList(list * head);
    void print()
    {
        list * tmp = lexems;
        printList(tmp);
    }
public:
    lexer()
    {
        capacity = 256;
        buf = new char (capacity);
        size = 0;
        buf[0] = 0;
        lexems = NULL;
        line = 1;
    }
    void start();
    ~lexer();
};

void lexer::getChar()
{
    c = getchar();
    if(size == capacity)
        resize();
    type = defineType(c);
    buf[size] = c;
    size++;
}

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

lexer::~lexer()
{
    freemem();
    delete[] buf;
}

void lexer::error()
{
    //printf("%s\n", buf);
    printf("error on line %d\n", line);
}

void lexer::resize()
{
    char * tmp = new char (capacity * 2);
    strncpy(tmp, buf, capacity);
    capacity *= 2;
    delete[] buf;
    buf = tmp;
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

void lexer::start()
{
    if(size == 1 && defineType(buf[0]) == dots){
        assign();
        return;
    }
    getChar();
    (c == '\n') ? line++: 0;
    if(type == sep){
        size = 0;
        start();
        return;
    }
    if(type == digit)
        number();
    else if(type == alpha)
        keyword();
    else if(type == special)
        ident();
    else if(type == op){
        lexems = addToList(buf, line, Q);
        size = 0;
        start();
        return;
    }
    else if(type == dots)
        assign();
    else if(type == quotes)
        quote();
    else if(c == EOF)
        print();
    else
        error();
}

void lexer::number()
{
    getChar();
    if(type == digit){
        number();
        return;
    }
    if(type == op || type == sep || type == eq){
        buf[size - 1] = 0;
        if(type == op || type == eq){
            buf[size] = c;
            buf[size + 1] = 0;
            lexems = addToList(buf, line, N);
            lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, N);
            (c == '\n') ? line++: 0;
        }
        size = 0;
        start();
        return;
    }
    error();
}

void lexer::indNum()
{
    getChar();
    if(type == digit)
        indNum();
    else if(type == idxr){
        buf[size - 1] = 0;
        buf[size] = c;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, N);
        lexems = addToList(&buf[size], line, IDX);
        size = 0;
        start();
    }
    else
        error();
}

void lexer::funcArg()
{
    getChar();
    if(type == funcr){
        buf[size - 1] = 0;
        buf[size] = c;
        buf[size + 1] = 0;
        lexems = addToList(buf, line, ARG);
        lexems = addToList(&buf[size], line, CLL);
        size = 0;
        start();
    }
    else if(type == sep){
        buf[size - 1] = 0;
        if(size != 1)
            lexems = addToList(buf, line, ARG);
        size = 0;
        funcArg();
    }
    else if(type == alpha || type == digit)
        funcArg();
    else
        error();
}

void lexer::index()
{
    getChar();
    if(type == digit)
        indNum();
    else
        error();
}

void lexer::funccall()
{
    getChar();
    if(type == digit || type == alpha)
        funcArg();
    else error();
}

void lexer::ident()
{
    getChar();
    if(type == digit || type == alpha)
        ident();
    else if(type == op||type == sep||type == eq||type == idxl||type == funcl){
        buf[size - 1] = 0;
        if(type == op || type == eq || type == idxl || type == funcl){
            buf[size] = c;
            buf[size + 1] = 0;
            lexems = addToList(buf, line, I);
            if(type == idxl && buf[0] == '$')
                lexems = addToList(&buf[size], line, IDX);
            else if(type == funcl && buf[0] == '?')
                lexems = addToList(&buf[size], line, CLL);
            else if(type == funcl || type == idxl){
                error();
                return;
            }
            else
                lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, I);
            (c == '\n') ? line++: 0;
        }
        size = 0;
        if(type == idxl)
            index();
        else if(type == funcl)
            funccall();
        else start();
    }
    else if(type == dots){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, I);
        size = 1;
        buf[0] = c;
        start();
    }
    else error();
}

void lexer::keyword()
{
    getChar();
    if(type == alpha){
        keyword();
        return;
    }
    if(type == op || type == sep || type == eq){
        buf[size - 1] = 0;
        if(type == op || type == eq){
            buf[size] = c;
            buf[size + 1] = 0;
            lexems = addToList(buf, line, K);
            lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, K);
            (c == '\n') ? line++: 0;
        }
        size = 0;
        start();
        return;
    }
    error();
}

void lexer::quote()
{
    getChar();
    if(type == quotes){
        buf[size] = 0;
        lexems = addToList(buf, line, S);
        size = 0;
        start();
        return;
    }
    else if(c != EOF){
        quote();
        return;
    }
    error();
}

void lexer::assign()
{
    getChar();
    if(type == eq){
        buf[size] = 0;
        lexems = addToList(buf, line, A);
        size = 0;
        start();
        return;
    }
    error();
}

int main(int argc, char ** argv){
    lexer a;
    a.start();
    return 0;
}
