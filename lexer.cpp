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
    "index"
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
    idxr
};

enum typesL
{
    N,
    I,
    K,
    A,
    S,
    Q,
    IDX
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
    if(c == ' ' || c == '\t' || c == '\n' || c == ';')
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
    return -1;
}

class lexer
{
    char * buf;
    int size;
    int capacity;
    int line;
    list * lexems;
    void freemem();
    void number();
    void ident();
    void keyword();
    void quote();
    void assign();
    void indNum();
    void index();
    void resize();
    void error();
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
    printf("zxc on %d\n", line);
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
    int c = getchar();
    buf[size] = c;
    int type = defineType(c);
    (c == '\n') ? line++: 0;
    if(type == sep){
        start();
        return;
    }
    size++;
    buf[size] = 0;
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
    int c = getchar();
    if(size == capacity)
        resize();
    int type = defineType(c);
    buf[size] = c;
    size++;
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

void lexer::index()
{
    int c = getchar();
    if(size == capacity)
        resize();
    int type = defineType(c);
    buf[size] = c;
    size++;
    if(type == digit)
        indNum();
    else error();
}

void lexer::ident()
{
    int c = getchar();
    if(size == capacity)
        resize();
    int type = defineType(c);
    buf[size] = c;
    size++;
    if(type == digit || type == alpha)
        ident();
    else if(type == op || type == sep || type == eq || type == idxl){
        buf[size - 1] = 0;
        if(type == op || type == eq || type == idxl){
            buf[size] = c;
            buf[size + 1] = 0;
            lexems = addToList(buf, line, I);
            type == idxl ? lexems = addToList(&buf[size], line, IDX) : lexems = addToList(&buf[size], line, Q);
        }
        else{
            lexems = addToList(buf, line, I);
            (c == '\n') ? line++: 0;
        }
        size = 0;
        (type == idxl) ? index() : start();
    }
    else if(type == dots){
        buf[size - 1] = 0;
        lexems = addToList(buf, line, I);
        size = 1;
        buf[0] = c;
        start();
    }
    else
        error();
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
    error();
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
    error();
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
    a.start();
    return 0;
}
