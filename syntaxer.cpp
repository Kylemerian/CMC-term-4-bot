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

class RPN
{
    varInfo var;
    RPNItem * rpn;
    RPNItem * stack;
    void freemem();
public:
    void exc(){
        RPNItem * tmp = rpn;
        while(tmp){
            tmp -> elem -> evaluate(&stack, &tmp, &var);
        }
    }
    RPN(RPNItem * arpn) : var(){
        stack = 0;
        rpn = arpn;
    }
    ~RPN(){
        freemem();
    }
};

class syntaxer
{
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
    void println_hdl();
    void pr_list_hdl();
    void pr_elem_hdl();
    void checkError(const char * str);
    void safeGetLex(const char * str);
    int isFunc(char * str);
    void addFuncToRPN(char * s);
    void addOperToRPN(char * s);
    void addUnOperToRPN(char * s);
    int equalStr(const char * a, const char * b) const;
    void reverse();
public:
    RPNItem * getRPN(){
        reverse();
        return rpn;
    } 
    void printRPN();
    void checkSeq(list * lexems);
    syntaxer();
};

void RPN::freemem()
{
    var.freemem();
    RPNItem * tmp = rpn;
    while(rpn){
        tmp = rpn;
        rpn = rpn -> next;
        delete tmp->elem;
        delete tmp;
    }
    tmp = stack;
    while(stack){
        tmp = stack;
        stack = stack -> next;
        delete tmp->elem;
        delete tmp;
    }
}

void syntaxer::printRPN()
{
    RPNItem * tmp = rpn;
    while(tmp){
        tmp -> elem -> print();
        tmp = tmp -> next;
    }
}

void syntaxer::reverse()
{
    RPNItem * rev = NULL;
    for(RPNItem * qq = rpn, *next_node; qq != NULL; qq = next_node){
        next_node = qq->next;
        qq -> next = rev;
        rev = qq;
    }
    rpn = rev;
}

void syntaxer::addOperToRPN(char * s)
{
    if(equalStr(s, "+"))
        addToRPN(new RPNFunPlus);
    if(equalStr(s, "-"))
        addToRPN(new RPNFunMinus);
    if(equalStr(s, "*"))
        addToRPN(new RPNFunMultiply);
    if(equalStr(s, "/"))
        addToRPN(new RPNFunDivision);
    if(equalStr(s, "%"))
        addToRPN(new RPNFunMod);
    if(equalStr(s, "or"))
        addToRPN(new RPNFunOr);
    if(equalStr(s, "and"))
        addToRPN(new RPNFunAnd);
    if(equalStr(s, "<"))
        addToRPN(new RPNFunLess);
    if(equalStr(s, ">"))
        addToRPN(new RPNFunMore);
    if(equalStr(s, "="))
        addToRPN(new RPNFunEqual);
}

void syntaxer::addUnOperToRPN(char * s)
{
    if(equalStr(s, "not"))
        addToRPN(new RPNFunNot);
    if(equalStr(s, "-"))
        addToRPN(new RPNFunUnoMinus);
}

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

syntaxer::syntaxer() //, opers()
{
    rpn = NULL;
    errline = 0;
    lexems = NULL;
    addToRPN(new RPNNoOp);
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
        char * tmp = lexems -> lex;
        safeGetLex("No operand in expression");
        exp_hdl();
        addOperToRPN(tmp);
    }
}

void syntaxer::exp5_hdl()
{
    exp4_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "and")){
        char * tmp = lexems -> lex;
        safeGetLex("No operand in expression");
        exp5_hdl();
        addOperToRPN(tmp);
    }
}

void syntaxer::exp4_hdl()
{
    exp3_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, ">") || equalStr(lexems -> lex, "<")
        || equalStr(lexems -> lex, "=")){
        
        char * tmp = lexems -> lex;
        safeGetLex("No operand in expression");
        exp4_hdl();
        addOperToRPN(tmp);
    }
}

void syntaxer::exp3_hdl()
{
    exp2_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "+") || equalStr(lexems -> lex, "-")){
        char * tmp = lexems -> lex;
        safeGetLex("No operand in expression");
        exp3_hdl();
        addOperToRPN(tmp);
    }
}

void syntaxer::exp2_hdl()
{
    exp1_hdl();
    if(!lexems)
        return;
    if(equalStr(lexems -> lex, "*") || equalStr(lexems -> lex, "/")
        || equalStr(lexems -> lex, "%")){
        
        char * tmp = lexems -> lex;
        safeGetLex("No operand in expression");
        exp2_hdl();
        addOperToRPN(tmp);
    }
}

void syntaxer::exp1_hdl()
{
    if(equalStr(lexems -> lex, "not") || equalStr(lexems -> lex, "-")){
        char * tmp = lexems -> lex;
        safeGetLex("No operand");
        exp0_hdl();
        addUnOperToRPN(tmp);
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
    addToRPN(new RPNNoOp);
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

void syntaxer::println_hdl()
{
    safeGetLex("No ; after println");
    if(!equalStr(lexems -> lex, ";"))
        throw error(lexems -> line, "No ; after statement");
    addToRPN(new RPNPrintln);
    errline = lexems -> line;
    lexems = lexems -> next;
}

void syntaxer::statement_hdl()
{
    if(equalStr(lexems -> lex, "while"))
        while_hdl();
    else if(equalStr(lexems -> lex, "print"))
        print_hdl();
    else if(equalStr(lexems -> lex, "println"))
        println_hdl();
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
        exit(0);
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
    lex.reverse();
    if(!lex.hasError())
        syntax.checkSeq(lex.getLexList());
    syntax.printRPN();
    printf("\n");
    RPN prog(syntax.getRPN());
    try{
        prog.exc();
    }
    catch(const char * s){
        printf("\n%s\n", s);
    }
    return 0;
}

