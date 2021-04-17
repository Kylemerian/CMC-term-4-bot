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
    int line;
    char * errmsg;
public:
    error(int aline, const char * msg);
    ~error();
    void printmsg()
    {
            printf("%s on line %d\n", errmsg, line);
    }
};

error::error(int aline, const char * msg)
{
    line = aline;
    errmsg = new char [strlen(msg) + 1];
    strncpy(errmsg, msg, strlen(msg));
    errmsg[strlen(msg)] = 0;
}

error::~error()
{
    delete[] errmsg;
}

class syntaxer
{
    int errline;
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
    void exp0_hdl();
    void exp1_hdl();
    void exp2_hdl();
    void exp3_hdl();
    void exp4_hdl();
    void exp5_hdl();
    void checkError(const char * str);
    void safeGetLex(const char * str);
    int equalStr(const char * a, const char * b) const;
public:
    void checkSeq(list * lexems);
    syntaxer();
};

syntaxer::syntaxer()
{
    errline = 0;
    lexems = NULL;
}

void syntaxer::checkError(const char * str)
{
    if(!lexems)
        throw error(errline, str);
}

int syntaxer::equalStr(const char * a, const char * b) const
{
    return !strcmp(a, b);
}

void syntaxer::safeGetLex(const char * str)
{
    if(!lexems -> next)
        throw error (lexems -> line, str);
    lexems = lexems -> next;
}

void syntaxer::exp_hdl()
{
    //printf("was in exp with %s\n", lexems -> lex);
    exp5_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "or")){
        safeGetLex("No operand in expression");
        exp_hdl();
    }
}

void syntaxer::exp5_hdl()
{
    //printf("was in exp5 with %s\n", lexems -> lex);
    exp4_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "and")){
        safeGetLex("No operand in expression");
        exp5_hdl();
    }
}

void syntaxer::exp4_hdl()
{
    //printf("was in exp4 with %s\n", lexems -> lex);
    exp3_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, ">") || equalStr(lexems -> lex, "<") 
        || equalStr(lexems -> lex, "=")){
        
        safeGetLex("No operand in expression");
        exp4_hdl();
    }
}

void syntaxer::exp3_hdl()
{
    //printf("was in exp3 with %s\n", lexems -> lex);
    exp2_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "+") || equalStr(lexems -> lex, "-")){
        safeGetLex("No operand in expression");
        exp3_hdl();
    }
}

void syntaxer::exp2_hdl()
{
    //printf("was in exp2 with %s\n", lexems -> lex);
    exp1_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "*") || equalStr(lexems -> lex, "/") 
        || equalStr(lexems -> lex, "%")){
        
        safeGetLex("No operand in expression");
        exp2_hdl();
    }
}

void syntaxer::exp1_hdl()
{
    //printf("was in exp1 with %s\n", lexems -> lex);
    if(equalStr(lexems -> lex, "not") || equalStr(lexems -> lex, "-"))
        safeGetLex("No operand");
    exp0_hdl();
}

void syntaxer::exp0_hdl()
{
    //printf("was in exp0 with %s\n", lexems -> lex);
    if(!equalStr(lexems -> lex, "(")){
        operand_hdl();
        return;
    }
    safeGetLex("No ) in expression");
    exp_hdl();
    checkError("No ) in expression");
    if(!equalStr(lexems -> lex, ")"))
        throw error(lexems -> line, "No ) in expression");
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::assign_hdl()
{
    var_hdl();
    checkError("No := in assign");
    if(!equalStr(lexems -> lex, ":="))
        throw error(lexems -> line, "No := in assign");
    safeGetLex("No expression after :=");
    exp_hdl();
    checkError("No ; after statement");
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems -> line, "No ; after statement");
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::var_hdl()
{
    errline = lexems -> line;
    lexems = lexems -> next;//could be problem
    if(!equalStr(lexems -> lex, "["))
        return;
    safeGetLex("No expression in []");
    exp_hdl();
    checkError("No ] in indexing");
    if(!equalStr(lexems -> lex, "]"))
        throw error(lexems -> line, "No ] in var indexing");
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::func_hdl()
{
    safeGetLex("No ( in function call");
    if(!equalStr(lexems -> lex, "("))
        throw error(lexems -> line, "No () in function call");
    safeGetLex("No ) in function call");
    if(equalStr(lexems -> lex, ")")){
        errline = lexems -> line;
        lexems = lexems -> next;
        return;
    }
    exp_hdl();
    checkError("No ) in function call");
    if(equalStr(lexems -> lex, ")")){
        errline = lexems -> line;
        lexems = lexems -> next;
        return;
    }
    exp_hdl();
    checkError("No ) in function call");
    if(!equalStr(lexems -> lex, ")"))
        throw error(lexems -> line, "No ) in function call");
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::operand_hdl()
{
    //printf("ope = %s\n", lexems -> lex);
    if(lexems -> type == N){
        errline = lexems -> line;
        lexems = lexems -> next; //TODO     safe operand or next statement
    }
    else if(isVar(lexems -> lex))
        var_hdl();
    else if(isFunc(lexems -> lex))
        func_hdl();
    else
        throw error(lexems -> line, "incorrect operand");
}

void syntaxer::if_hdl()
{
    safeGetLex("No expression");
    exp_hdl();
    checkError("No then in if statement");
    if(!equalStr(lexems -> lex, "then"))
        throw error(lexems -> line, "No then in if statement");
    safeGetLex("No { in if statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems -> line, "No { in if statement");
    safeGetLex("No } in if statement");
    body_hdl();
    checkError("No } in if statement");
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems -> line, "No } in if statement");
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::while_hdl()
{
    safeGetLex("No expression");
    exp_hdl();
    checkError("No do in while statement");
    if(!equalStr(lexems -> lex, "do"))
        throw error(lexems -> line, "No do in while statement");
    safeGetLex("No { in while statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems -> line, "No { in while statement");
    safeGetLex("No } in while statement");
    body_hdl();
    checkError("No } in while statement");
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems -> line, "No } in while statement");
    errline = lexems -> line;
    lexems = lexems -> next;
}

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
        checkError("No ; after statement");
        if(!equalStr(lexems -> lex, ";")){
            printf("%s \n", lexems -> lex);
            throw error(lexems -> line, "No ; after statement");
        }
        errline = lexems -> line;
        lexems = lexems -> next;
    }
    else if(isVar(lexems -> lex))
        assign_hdl();
    else if(!equalStr(lexems -> lex, "}")){
        throw error(lexems -> line, "Incorrect statement");
    }
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
    lexer lex;
    syntaxer syntax;
    while((c = getchar()) != EOF)
        lex.sendChar(c);
    c = ' ';
    lex.sendChar(c);
    //lex.print();
    lex.reverse();
    if(!lex.hasError())
        syntax.checkSeq(lex.getLexList());
    return 0;
}

