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
    "operator"
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
    quotes
};

enum typesL
{
    N,
    I,
    K,
    A,
    S,
    O
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
    if(c == ' ' || c == '\t' || c == '\n')
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
    return -1;
}

class lexer
{
    char * buf;
    int size;
    int line;
    list * lexems;
    void number();
    void ident();
    void keyword();
    void quote();
    void assign();
    list * addToList(char * key, int line, int type);
public:
    lexer()
    {
        buf = (char *)malloc(1024);
        size = 0;
        buf[0] = 0;
        lexems = NULL;
        line = 1;
    }
    void start();
    void print()
    {
        list * tmp = lexems;
        printList(tmp);
    }
    void printList(list * head)
    {
        if (head != NULL) {
            printList(head -> next);
            printf("line %2d type = %13s   %s\n", head -> line, typeLex[head -> type], head -> lex);
        }
    }
};

list * lexer::addToList(char * key, int line, int type)
{
    list * tmp = (list *)malloc(sizeof(*tmp));
    tmp -> line = line;
    tmp -> type = type;
    tmp -> lex = (char *)malloc(strlen(key) + 1);
    strncpy(tmp -> lex, key, strlen(key));
    tmp -> lex[strlen(key)] = 0;
    tmp -> next = lexems;
    return tmp;
}

void lexer::start()
{
    int c = getchar();
    buf[size] = c;
    int type = defineType(c);
    if(c == '\n')
        line++;
    if(type == sep){
        start();
        return;
    }
    size++;
    buf[size] = 0;
    switch (type){
        case digit:
            number();
            break;
        case alpha:
            keyword();
            break; 
        case special:
            ident();
            break;
        case op:
            lexems = addToList(buf, line, O);
            size = 0;
            start();
            break;
        case dots:
            assign();
            break;
        case quotes:
            quote();
            break;
        default:
            print();
            return;
    }
}

void lexer::number()
{
    int c = getchar();
    int type = defineType(c);
    buf[size] = c;
    size++;
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
            lexems = addToList(&buf[size], line, O);
        }
        else{
            lexems = addToList(buf, line, N);
            line++;
        }
        size = 0;
        start();
        return;
    }
    return;
}

void lexer::ident()
{
    int c = getchar();
    int type = defineType(c);
    buf[size] = c;
    size++;
    if(type == digit || type == alpha){
        ident();
        return;
    }
    if(type == op || type == sep || type == eq){
        buf[size - 1] = 0;
        if(type == op || type == eq){
            buf[size] = c;
            buf[size + 1] = 0;
            lexems = addToList(buf, line, I);
            lexems = addToList(&buf[size], line, O);
        }
        else{
            lexems = addToList(buf, line, I);
            line++;
        }
        size = 0;
        start();
        return;
    }
    return;
}

void lexer::keyword()
{
    int c = getchar();
    int type = defineType(c);
    buf[size] = c;
    size++;
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
            lexems = addToList(&buf[size], line, O);
        }
        else{
            lexems = addToList(buf, line, K);
            line++;
        }
        size = 0;
        start();
        return;
    }
    return;
}

void lexer::quote()
{
    int c = getchar();
    int type = defineType(c);
    buf[size] = c;
    size++;
    if(type == quotes){
        buf[size] = 0;
        lexems = addToList(buf, line, S);
        size = 0;
        start();
    }
    else
        quote();
}

void lexer::assign()
{
    int c = getchar();
    int type = defineType(c);
    buf[size] = c;
    size++;
    if(type == eq){
        buf[size] = 0;
        lexems = addToList(buf, line, A);
        size = 0;
        start();
        return;
    }
    return;
}

int main(int argc, char ** argv){
    int fd[2];
    fd[0] = open(argv[1], O_RDONLY);
    //fd[1] = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0777);
    dup2(fd[0], 0);
    //dup2(fd[1], 1);
    //close(fd[1]);
    close(fd[0]);
    lexer a;
    a.start();
    return 0;
}
