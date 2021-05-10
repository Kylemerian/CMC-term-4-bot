#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "rpn.h"
#include "vars.h"

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
    void printmsg(){
            printf("%s on line %d\n", errmsg, line);
    }
    error(const error & a);
};

error::error(const error & a)
{
    line = a.line;
    errmsg = new char [strlen(a.errmsg) + 1];
    strncpy(errmsg, a.errmsg, strlen(a.errmsg));
    errmsg[strlen(a.errmsg)] = 0;
}

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
    varInfo var;
    RPNItem * stack;
    RPNItem * rpn;
    void addToRPN(RPNElem * a);
    int errline;
    list * lexems;
    void exp_hdl();
    void assign_hdl();
    void var_hdl();
    void operand_hdl();
    void statement_hdl();
    void while_hdl();
    void if_hdl();
    void body_hdl();
    void func_hdl();
    void exp0_hdl();
    void exp1_hdl();
    void exp2_hdl();
    void exp3_hdl();
    void exp4_hdl();
    void exp5_hdl();
    void print_hdl();
    void pr_list_hdl();
    void pr_elem_hdl();
    void checkError(const char * str);
    void safeGetLex(const char * str);
    int isFunc(char * str);
    void addFuncToRPN(char * s);
    int equalStr(const char * a, const char * b) const;
public:
    /**/void printRPN(){
        while(rpn){
            rpn->elem->print();
            rpn = rpn -> next;
        }
    }
    void checkSeq(list * lexems);
    syntaxer();
};

void syntaxer::addFuncToRPN(char * s)
{
    if(equalStr(s, "?sell"))
        addToRPN(new RPNSell);
    if(equalStr(s, "?buy"))
        addToRPN(new RPNBuy);
    if(equalStr(s, "?prod"))
        addToRPN(new RPNProd);
    if(equalStr(s, "?endturn"))
        addToRPN(new RPNEndturn);
}

int syntaxer::isFunc(char * str)
{
    return equalStr(str, "?sell") || equalStr(str, "?buy") ||
        equalStr(str, "?prod") || equalStr(str, "?endturn"); 
}

void syntaxer::addToRPN(RPNElem * a)
{
    RPNItem * tmp = new RPNItem;
    tmp -> elem = a;
    tmp -> next = rpn;
    rpn = tmp;
}

syntaxer::syntaxer() : var()
{
    rpn = NULL;
    addToRPN(new RPNNoOp);
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
    exp5_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "or")){
        safeGetLex("No operand in expression");
        exp_hdl();
        addToRPN(new RPNFunOr);
    }
}

void syntaxer::exp5_hdl()
{
    exp4_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "and")){
        safeGetLex("No operand in expression");
        exp5_hdl();
        addToRPN(new RPNFunAnd);
    }
}

void syntaxer::exp4_hdl()
{
    exp3_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, ">")){
        safeGetLex("No operand in expression");
        exp4_hdl();
        addToRPN(new RPNFunMore);
    }
    else if(equalStr(lexems -> lex, "<")){
        safeGetLex("No operand in expression");
        exp4_hdl();
        addToRPN(new RPNFunLess);
    }
    else if(equalStr(lexems -> lex, "=")){
        safeGetLex("No operand in expression");
        exp4_hdl();
        addToRPN(new RPNFunEqual);
    }
}

void syntaxer::exp3_hdl()
{
    exp2_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "+")){
        safeGetLex("No operand in expression");
        exp3_hdl();
        addToRPN(new RPNFunPlus);
    }
    else if(equalStr(lexems -> lex, "-")){
        safeGetLex("No operand in expression");
        exp3_hdl();
        addToRPN(new RPNFunMinus);
    }
}

void syntaxer::exp2_hdl()
{
    exp1_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "*")){
        safeGetLex("No operand in expression");
        exp2_hdl();
        addToRPN(new RPNFunMultiply);
    }
    else if(equalStr(lexems -> lex, "/")){
        safeGetLex("No operand in expression");
        exp2_hdl();
        addToRPN(new RPNFunDivision);
    }
    else if(equalStr(lexems -> lex, "%")){
        safeGetLex("No operand in expression");
        exp2_hdl();
        addToRPN(new RPNFunMod);
    }
}

void syntaxer::exp1_hdl()
{
    if(equalStr(lexems -> lex, "not")){
        safeGetLex("No operand");
        exp0_hdl();
        addToRPN(new RPNFunNot);
    }
    else if(equalStr(lexems -> lex, "-")){
        safeGetLex("No operand");
        exp0_hdl();
        addToRPN(new RPNFunUnoMinus);
    }
    else
        exp0_hdl();
}

void syntaxer::exp0_hdl()
{
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
    lexems = lexems -> next;
    if(!equalStr(lexems -> lex, "[")){
        addToRPN(new RPNInt(0));
        return;
    }
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
    if(lexems -> type == N){
        addToRPN(new RPNInt(atoi(lexems -> lex)));
        errline = lexems -> line;
        lexems = lexems -> next; 
    }
    else if(isVar(lexems -> lex)){
        char * tmp = lexems -> lex;
        var_hdl();
        addToRPN(new RPNVarAddr(tmp));
        addToRPN(new RPNFunVar);
    }
    else if(isFunc(lexems -> lex)){
        char * tmp = lexems -> lex;
        func_hdl();
        addFuncToRPN(tmp);
    }
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
    RPNLabel * lab1 = new RPNLabel;
    addToRPN(lab1);
    addToRPN(new RPNOpGoFalse);
    safeGetLex("No { in if statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems -> line, "No { in if statement");
    safeGetLex("No } in if statement");
    body_hdl();
    checkError("No } in if statement");
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems -> line, "No } in if statement");
    addToRPN(new RPNNoOp);
    lab1->set(rpn);
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::while_hdl()
{
    safeGetLex("No expression");
    RPNItem * beforeExp = rpn;
    exp_hdl();
    checkError("No do in while statement");
    if(!equalStr(lexems -> lex, "do"))
        throw error(lexems -> line, "No do in while statement");
    RPNLabel * lab1 = new RPNLabel; 
    addToRPN(lab1);
    addToRPN(new RPNOpGoFalse);
    safeGetLex("No { in while statement");
    if(!equalStr(lexems -> lex, "{"))
        throw error(lexems -> line, "No { in while statement");
    safeGetLex("No } in while statement");
    body_hdl();
    checkError("No } in while statement");
    if(!equalStr(lexems -> lex, "}"))
        throw error(lexems -> line, "No } in while statement");
    addToRPN(new RPNLabel(beforeExp));
    addToRPN(new RPNOpGo);
    addToRPN(new RPNNoOp);
    lab1->set(rpn);
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::pr_list_hdl()
{
    pr_elem_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, ",")){
        safeGetLex("No arg after ,");
        pr_list_hdl();
    }
}
    
void syntaxer::pr_elem_hdl()
{
    if(lexems -> type == Str){
        addToRPN(new RPNString(lexems -> lex));
        addToRPN(new RPNPrint);
        errline = lexems -> line;
        lexems = lexems -> next;
        return;
    }
    exp_hdl();
    addToRPN(new RPNPrint);
}

void syntaxer::print_hdl()
{
    safeGetLex("No args for print");
    pr_list_hdl();
    checkError("No ; after statement");
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems -> line, "No ; after statement");
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
    else if(equalStr(lexems -> lex, "print"))
        print_hdl();
    else if(equalStr(lexems -> lex, "if"))
        if_hdl();
    else if(isFunc(lexems -> lex)){
        char * tmp = lexems -> lex;
        func_hdl();
        addFuncToRPN(tmp);
        checkError("No ; after statement");
        if(!equalStr(lexems -> lex, ";"))
            throw error(lexems -> line, "No ; after statement");
        errline = lexems -> line;
        lexems = lexems -> next;
    }
    else if(isVar(lexems -> lex)){
        char * tmp = lexems -> lex;
        var_hdl();
        addToRPN(new RPNVarAddr(tmp));
        assign_hdl();
        addToRPN(new RPNFunAssign);
    }
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
    syntax.printRPN();
    return 0;
}

