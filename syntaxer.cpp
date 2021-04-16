#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"

int isFunc(char * a)
{
    return a[0] == '?';
}

int isVar(char * a)
{
    return a[0] == '$';
}

class error
{
    list * curlex;
    char * errmsg;
public:
    error(list * lex, const char * msg);
    ~error();
    void printmsg()
    {
        if(curlex)
            printf("%s on line %d\n", errmsg, curlex -> line);
        else
            printf("%s\n", errmsg);
    }
};

error::error(list * lex, const char * msg)
{
    curlex = lex;
    errmsg = new char (strlen(msg) + 1);
    strncpy(errmsg, msg, strlen(msg));
    errmsg[strlen(msg)] = 0;
}

error::~error()
{
    delete[] errmsg;
}

class syntaxer
{
    list * lexems;
    void exp_hdl();
    void assign_hdl();
    void var_hdl();
    void operand_hdl();
    void statement_hdl();
    void while_hdl();
    void if_hdl();
    void buy_sell_hdl();
    void prod_hdl();
    void turn_hdl();
    void body_hdl();
    void func_hdl();
    void safeGetLex(const char * str);
    int equalStr(const char * a, const char * b) const;
public:
    void checkSeq(list * lexems);
    syntaxer();
};

syntaxer::syntaxer()
{
    lexems = NULL;
}

int syntaxer::equalStr(const char * a, const char * b) const
{
    return !strcmp(a, b);
}

void syntaxer::safeGetLex(const char * str)
{
    if(!lexems -> next)
        throw error (lexems, str);
    lexems = lexems -> next;
}

void syntaxer::exp_hdl()
{
    operand_hdl();
    if(lexems -> type == H){
        safeGetLex("No operand in expression");
        exp_hdl();
    }
}

void syntaxer::assign_hdl()
{
    var_hdl();
    if(!equalStr(lexems -> lex, ":="))
        throw error(lexems, "No := in assign");
    safeGetLex("No expression after :=");
    exp_hdl();
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems, "No ; after statement");
    lexems = lexems -> next;
}

void syntaxer::var_hdl()
{
    lexems = lexems -> next; //could be problem
    if(!equalStr(lexems -> lex, "["))
        return;
    safeGetLex("No expression in []");
    exp_hdl();
    if(!equalStr(lexems -> lex, "]"))
        throw error(lexems, "No ] in var indexing");
    lexems = lexems -> next;
}

void syntaxer::func_hdl()
{
    safeGetLex("No ( in function call");
    if(!equalStr(lexems -> lex, "("))
        throw error(lexems, "No () in function call");
    safeGetLex("No ) in function call");
    if(equalStr(lexems -> lex, ")")){
        lexems = lexems -> next;
        return;
    }
    exp_hdl();
    if(equalStr(lexems -> lex, ")")){
        lexems = lexems -> next;
        return;
    }
    exp_hdl();
    if(!equalStr(lexems -> lex, ")"))
        throw error(lexems, "No ) in function call");
    lexems = lexems -> next;
}

void syntaxer::operand_hdl()
{
    if(lexems -> type == N){
        lexems = lexems -> next; //TODO     safe operand or next statement
    }
    else if(isVar(lexems -> lex))
        var_hdl();
    else if(isFunc(lexems -> lex))
        func_hdl();
    else
        throw error(lexems, "incorrect operand");
}

void syntaxer::if_hdl()
{
    safeGetLex("No expression");
    exp_hdl();
    if(!equalStr(lexems -> lex, "then"))
        throw error(lexems, "No do in if statement");
    safeGetLex("No { in if statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems, "No { in if statement");
    safeGetLex("No } in if statement");
    statement_hdl();
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems, "No } in if statement");
    lexems = lexems -> next;
}

void syntaxer::while_hdl()
{
    safeGetLex("No expression");
    exp_hdl();
    if(!equalStr(lexems -> lex, "do"))
        throw error(lexems, "No do in while statement");
    safeGetLex("No { in while statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems, "No { in while statement");
    safeGetLex("No } in while statement");
    body_hdl();
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems, "No } in while statement");
    lexems = lexems -> next;
}
/*
void syntaxer::buy_sell_hdl()
{
    safeGetLex("No arg in function call");
    exp_hdl();
    exp_hdl();
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems, "No ; after statement");
    lexems = lexems -> next;
}

void syntaxer::prod_hdl()
{
    safeGetLex("No arg in function call");
    exp_hdl();
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems, "No ; after statement");
    lexems = lexems -> next;
}

void syntaxer::turn_hdl()
{
    safeGetLex("No ; after statement");
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems, "No ; after statement");
    lexems = lexems -> next;
}
*/
void syntaxer::body_hdl()
{
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "}"))
        return;
    if(lexems){
        statement_hdl();
        body_hdl();
    }
}

void syntaxer::statement_hdl()
{
    if(equalStr(lexems -> lex, "while"))
        while_hdl();
    else if(equalStr(lexems -> lex, "if"))
        if_hdl();
    else if(isFunc(lexems -> lex)){
        func_hdl();
        if(!equalStr(lexems -> lex, ";")){
            printf("%s \n", lexems -> lex);
            throw error(lexems, "No ;;;; after statement");
        }
        lexems = lexems -> next;
    }
    /*else if(equalStr(lexems -> lex, "?buy") || equalStr(lexems -> lex, "?sell"))
        buy_sell_hdl();
    else if(equalStr(lexems -> lex, "?prod"))
        prod_hdl();
    else if(equalStr(lexems -> lex, "?endturn"))
        turn_hdl();*/
    else if(isVar(lexems -> lex))
        assign_hdl();
    else if(!equalStr(lexems -> lex, "}"))
        throw error(lexems, "Incorrect statement");
}

void syntaxer::checkSeq(list * qlexems)
{
    lexems = qlexems;
    int err = 0;
    try{
        body_hdl();
    }
    catch(error & a)
    {
        err = 1;
        a.printmsg();
    }
    if(!err)
        printf("OK\n");
}

int main()
{
    int c;
    lexer obj;
    syntaxer syntax;
    while((c = getchar()) != EOF)
        obj.sendChar(c);
    c = ' ';
    obj.sendChar(c);
    obj.print();
    syntax.checkSeq(obj.getLexList());
    return 0;
}

