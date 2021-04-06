#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"

int isFunc(char * a)
{
    return a[0] == '?';
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

void syntaxer::exp_hdl() //OK
{
    //printf("was here in exp with %s\n", lexems -> lex);
    operand_hdl();
    if(lexems -> type == H){
        if(lexems) lexems = lexems -> next;
        else throw error(lexems, "No operand after operator in expression");
        exp_hdl();
    }   
}

void syntaxer::assign_hdl() //OK
{
    var_hdl();
    //printf("was here in assign with %s\n", lexems -> lex);
    if(equalStr(lexems ->lex, ":=")){
        if(lexems) lexems = lexems -> next;
        else throw error(lexems, "No expression after :=");
        exp_hdl();
        lexems = lexems -> next;
    }
    else
        throw error(lexems, "No \":=\" in assigning");


}

void syntaxer::var_hdl()   //OK
{
    //printf("was here in var with %s\n", lexems -> lex);
    if(lexems -> type == I){
        if(equalStr(lexems -> next ->lex, "[")){
            lexems = lexems -> next;
            if(lexems) lexems = lexems -> next;
            else throw error(lexems, "No ] in var with index");
            exp_hdl();
            //printf("was here in var with %s\n", lexems -> lex);
            if(equalStr(lexems ->lex, "]")){
                lexems = lexems -> next;
                return;
            }
            else
                throw error(lexems, "No ] in var with index");
        }
        else{
            lexems = lexems -> next;
            return;
        }
    }
    else
        throw error(lexems, "There must be an operand");

}

void syntaxer::operand_hdl()    //OK   
{
    //printf("was here in oper with %s\n", lexems -> lex);
    if(!lexems)
        throw error(lexems, "No expression");
    if(lexems -> type == N)
        lexems = lexems -> next;
    else if(isFunc(lexems -> lex)){
        if(lexems) lexems = lexems -> next;
        else throw error(lexems, "No () in func call");
        if(equalStr(lexems -> lex, "(")){
            if(lexems) lexems = lexems -> next;
            else throw error(lexems, "No ) in func call");
            if(!equalStr(lexems -> lex, ")"))
                exp_hdl();
            if(!equalStr(lexems -> lex, ")"))
                exp_hdl();
            //printf("was here in oper with %s\n", lexems -> lex);
            if(equalStr(lexems -> lex, ")")){
                lexems = lexems -> next;
                return;
            }
            else
                throw error(lexems, "No ) in func call or more than 2 args");
        }
        else
            throw error(lexems, "No ( in func call");
    }
    else
        var_hdl();
}

void syntaxer::if_hdl() //OK
{
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No condition in if statement");
    exp_hdl();
    //printf("was here in if with %s\n", lexems -> lex);
    if(equalStr(lexems -> lex, "then")){
        if(lexems) lexems = lexems -> next;
        else throw error(lexems, "No {} in if statement");
        if(equalStr(lexems -> lex, "{")){
            if(lexems) lexems = lexems -> next;
            else throw error(lexems, "No } in if statement");
            statement_hdl();
            //printf("was here in if need } with %s\n", lexems -> lex);
            if(equalStr(lexems -> lex, "}")){
                lexems = lexems -> next;
                //printf("was here in if } with %s\n", lexems -> lex);
                return;
            }
            else
                throw error(lexems, "No } in if statement");
        }
        else
            throw error(lexems, "No { in if statement");
    }
    else
        throw error(lexems, "No \"then\" in if statement");
}

void syntaxer::while_hdl()  //OK
{
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No condition in while statement");
    exp_hdl();
    //printf("was here in while with %s\n", lexems -> lex);
    if(equalStr(lexems -> lex, "do")){
        if(lexems) lexems = lexems -> next;
        else throw error(lexems, "No {} in while statement");
        if(equalStr(lexems -> lex, "{")){
            if(lexems) lexems = lexems -> next;
            else throw error(lexems, "No } in while statement");
            statement_hdl();
            //printf("was here in while need } with %s\n", lexems -> lex);
            if(equalStr(lexems -> lex, "}")){
                lexems = lexems -> next;
                //printf("was here in if } with %s\n", lexems -> lex);
                return;
            }
            else
                throw error(lexems, "No } in while statement");
        }
        else
            throw error(lexems, "No { in while statement");
    }
    else
        throw error(lexems, "No \"do\" in while statement");
}

void syntaxer::buy_sell_hdl()
{
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No args after func call");
    exp_hdl();
    exp_hdl();
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No ; after statement");
}

void syntaxer::prod_hdl()
{
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No args after func call");
    exp_hdl();
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No ; after statement");
}

void syntaxer::turn_hdl()
{
    if(lexems) lexems = lexems -> next;
    else throw error(lexems, "No ; after statement");
    lexems = lexems -> next;
}

void syntaxer::statement_hdl() //OK
{
    if(!lexems)
        throw error(lexems, "Wrong statement");
    if(equalStr(lexems -> lex, "while"))
        while_hdl();
    else if(equalStr(lexems -> lex, "if"))
        //printf("was in state with %s\n", lexems -> lex);
        if_hdl();
    else if(equalStr(lexems -> lex, "buy") || equalStr(lexems -> lex, "sell"))
        buy_sell_hdl();
    else if(equalStr(lexems -> lex, "prod"))
        prod_hdl();
    else if(equalStr(lexems -> lex, "endturn"))
        turn_hdl(); 
    else if(lexems -> type == I)
        assign_hdl();
}

void syntaxer::checkSeq(list * qlexems)
{
    lexems = qlexems;
    int err = 0;
    try{
        while(lexems){/**/
            statement_hdl();
            //printf("was here in checkseq with %s\n", lexems -> lex);
        }
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

