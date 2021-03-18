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
    Q
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
    int capacity;
    int line;
    list * lexems;
    void number();
    void ident();
    void keyword();
    void quote();
    void assign();
    void resize();
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
        buf = (char *)malloc(capacity);
        size = 0;
        buf[0] = 0;
        lexems = NULL;
        line = 1;
    }
    void start();
    ~lexer()
    {
        free(buf);
    }
};

void lexer::resize()
{
    char * tmp = (char *)malloc(capacity * 2);
    strncpy(tmp, buf, capacity);
    capacity *= 2;
    free(buf);
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
            lexems = addToList(buf, line, Q);
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
    if(size == capacity)
        resize();
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
            lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, N);
            if(c == '\n')
                line++;
        }
        size = 0;
        start();
        return;
    }
    //throw line;
}

void lexer::ident()
{
    int c = getchar();
    if(size == capacity)
        resize();
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
            lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, I);
            if(c == '\n')
                line++;
        }
        size = 0;
        start();
        return;
    }
    //throw line;
}

void lexer::keyword()
{
    int c = getchar();
    if(size == capacity)
        resize();
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
            lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, K);
            if(c == '\n')
                line++;
        }
        size = 0;
        start();
        return;
    }
    //throw line;
}

void lexer::quote()
{
    int c = getchar();
    if(size == capacity)
        resize();
    int type = defineType(c);
    buf[size] = c;
    size++;
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
    throw line;
}

void lexer::assign()
{
    int c = getchar();
    if(size == capacity)
        resize();
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
    throw line;
}

int main(int argc, char ** argv){
    int fd[2];
    fd[0] = open(argv[1], O_RDONLY);
    fd[1] = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0777);
    dup2(fd[0], 0);
    dup2(fd[1], 1);
    close(fd[1]);
    close(fd[0]);
    lexer a;
    try{
        a.start();
    }
    catch(int nm){
        printf("incorrect on %d\n", nm);
    }
    return 0;
}
