#ifndef RPN_H
#define RPN_H
#include "vars.h"

class RPNElem;

struct RPNItem
{
    RPNItem * next;
    RPNElem * elem;
};

class RPNElem
{
public:
    virtual ~RPNElem(){};
    virtual void evaluate(RPNItem ** stack, RPNItem ** curCmd, varInfo * vars) const = 0;
    virtual void print() const = 0;
protected:
    static void push(RPNItem ** stack, RPNElem * elem){
        RPNItem * tmp = new RPNItem;
        tmp -> elem = elem;
        tmp -> next = *stack;
        *stack = tmp;
    }
    static RPNElem * pop(RPNItem ** stack){
        if (*stack){
            RPNElem *elem = (*stack) -> elem;
            RPNItem *tmp = *stack;
            *stack = (*stack) -> next;
            delete tmp;
            return elem;
        }
        return 0;
    }
};

class RPNConst : public RPNElem
{
public:
    virtual ~RPNConst(){};
    virtual RPNElem * clone() const = 0;
    void evaluate(RPNItem ** stack, RPNItem ** curCmd, varInfo * vars) const{
        push(stack, clone());
        *curCmd = (*curCmd) -> next;
    }
    void print() const = 0;
};

class RPNFunction : public RPNElem
{
public:
virtual ~RPNFunction(){};
    virtual RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const = 0;
    void evaluate(RPNItem ** stack, RPNItem ** curCmd, varInfo * vars) const{
        RPNElem * res = evaluateFun(stack, vars);
        if(res)
            push(stack, res);
        *curCmd = (*curCmd)->next;
    }
};

class RPNLabel : public RPNConst
{
    RPNItem * value;
public:
    RPNLabel(RPNItem * a = NULL){
        value = a;
    }
    virtual ~RPNLabel(){};
    virtual RPNElem * clone() const{
        return new RPNLabel(value);
    }
    RPNItem * get() const{
        return value;
    }
    void set(RPNItem * lab){
        value = lab;
    }
    void print() const{
        printf("&");
        value->elem->print();
    }
};

class RPNInt : public RPNConst
{
    int value;
public:
    RPNInt(int a){
        value = a;
    }
    virtual ~RPNInt(){};
    virtual RPNElem * clone() const{
        return new RPNInt(value);
    }
    int get() const{
        return value;
    }
    void print() const{
        printf("%d ", value);
    }
};

class RPNOpGo : public RPNElem
{
public:
    RPNOpGo(){};
    virtual ~RPNOpGo(){};
    void evaluate(RPNItem ** stack, RPNItem ** curCmd, varInfo * vars) const{
        RPNElem* op1 = pop(stack);
        RPNLabel * lab = dynamic_cast<RPNLabel *>(op1);
        if(!lab)
            throw "OpGo";
        RPNItem * addr = lab->get();
        *curCmd = addr;
        delete op1;      
    }
    void print() const{
        printf("! ");
    }
};

class RPNOpGoFalse : public RPNElem
{
public:
	RPNOpGoFalse(){}
	virtual ~RPNOpGoFalse(){}
	void evaluate(RPNItem ** stack, RPNItem ** cur_cmd, varInfo * vars) const{
		RPNElem *op1 = pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel *>(op1);
		if (!lab)
            throw "OpGoFalse";
		RPNElem *op2 = pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
		if (!i2)
            throw "OpGoFalse";
		if (!i2->get()){
			RPNItem *addr = lab->get();
			*cur_cmd = addr;
		}
        else
			*cur_cmd = (*cur_cmd)->next;  
		delete op1;
		delete op2;
	}
    void print() const{
        printf("!F ");
    }
};

class RPNString : public RPNConst
{
    char * value;
public:
    RPNString(){}
	virtual ~RPNString(){}
	RPNString(char * a){
        value = a;
    }
	virtual RPNElem * clone() const{
        return new RPNString(value);
    }
	char * get() const {
        return value;
    }
    void print() const{
        printf("%s ", value);
    }
};


class RPNVarAddr : public RPNConst
{
//name
    char * value;
public:
    RPNVarAddr(){}
    virtual ~RPNVarAddr(){}
    RPNVarAddr(char * a){
        value = a;
    }
    virtual RPNElem* clone() const{
        return new RPNVarAddr(value);
    }
    char * get() const {
        return value;
    }
    void print() const{
        printf("%s ", value);
    }
};

class RPNFunVar : public RPNFunction
{
public:
    RPNFunVar(){}
    virtual ~RPNFunVar(){}
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        //op1 -> print();
        RPNVarAddr * i1 = dynamic_cast<RPNVarAddr *>(op1);
        if(!i1)
            throw "FunVar";
        RPNElem * op2 = pop(stack);
        //op2 -> print();
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "FunVar";
        int res = vars -> findVar(i1 -> get(), i2 -> get());
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("[] ");
    }
};

class RPNSell : public RPNFunction
{
public:
    RPNSell(){};
    virtual ~RPNSell(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Sell";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Sell";
        /**/
        delete op1;
        delete op2;
        return 0;
    }
    void print() const{
        printf("?sell ");
    }
};

class RPNBuy : public RPNFunction
{
public:
    RPNBuy(){};
    virtual ~RPNBuy(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Buy";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Buy";
        /**/
        delete op1;
        delete op2;
        return 0;
    }
    void print() const{
        printf("?buy ");
    }
};

class RPNEndturn : public RPNFunction
{
public:
    RPNEndturn(){};
    virtual ~RPNEndturn(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        /**/
        return 0;
    }
    void print() const{
        printf("?endturn ");
    }
};

class RPNProd : public RPNFunction
{
public:
	RPNProd(){}
	virtual ~RPNProd(){}
	RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const 
	{
		RPNElem * op1 = pop(stack); 
		RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
		if (!i1)
            throw "Prod";
		/**/
		delete op1;
		return 0;
	}
    void print() const{
        printf("?prod ");
    }
};

class RPNFunPlus : public RPNFunction
{
public:
    RPNFunPlus() {};
    virtual ~RPNFunPlus(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Plus";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Plus";
        int res = i1 -> get() + i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("+ ");
    }
};

class RPNFunUnoMinus : public RPNFunction{
public:
    RPNFunUnoMinus(){}
    virtual ~RPNFunUnoMinus(){}
    RPNElem* evaluateFun(RPNItem ** stack, varInfo * vars) const{
        	RPNElem *op1 = pop(stack);
        	RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        	if (!i1)
                throw "Minus";
        	int res = -(i1 -> get());
       	 	delete op1;
        	return new RPNInt(res);
    }
    void print() const{
        printf("- ");
    }
};

class RPNFunMinus : public RPNFunction
{
public:
    RPNFunMinus() {};
    virtual ~RPNFunMinus(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Minus";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Minus";
        int res = i2 -> get() - i1 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("- ");
    }
};

class RPNFunMultiply : public RPNFunction
{
public:
    RPNFunMultiply() {};
    virtual ~RPNFunMultiply(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Multiply";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Multiply";
        int res = i1 -> get() * i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("* ");
    }
};

class RPNFunDivision : public RPNFunction
{
public:
    RPNFunDivision() {};
    virtual ~RPNFunDivision(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Division";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Divsion";
        int res = i2 -> get() / i1 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("/ ");
    }
};

class RPNFunMod : public RPNFunction
{
public:
    RPNFunMod() {};
    virtual ~RPNFunMod(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Mod";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Mod";
        int res = i2 -> get() % i1 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("%% ");
    }
};

class RPNFunNot : public RPNFunction
{
public:
    RPNFunNot() {};
    virtual ~RPNFunNot(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Not";
        int res = !(i1 -> get());
        delete op1;
        return new RPNInt(res);
    }
    void print() const{
        printf("not ");
    }
};

class RPNFunAnd : public RPNFunction
{
public:
    RPNFunAnd() {};
    virtual ~RPNFunAnd(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "And";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "And";
        int res = i1 -> get() && i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("and ");
    }
};

class RPNFunOr : public RPNFunction
{
public:
    RPNFunOr() {};
    virtual ~RPNFunOr(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Or";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Or";
        int res = i1 -> get() || i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("or ");
    }
};

class RPNFunEqual : public RPNFunction
{
public:
    RPNFunEqual() {};
    virtual ~RPNFunEqual(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Equal";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Equal";
        int res = i1 -> get() == i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("= ");
    }
};

class RPNFunLess : public RPNFunction
{
public:
    RPNFunLess() {};
    virtual ~RPNFunLess(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Less";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "Less";
        int res = i2 -> get() < i1 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("< ");
    }
};

class RPNFunMore : public RPNFunction
{
public:
    RPNFunMore() {};
    virtual ~RPNFunMore(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "More";
        RPNElem * op2 = pop(stack);
        RPNInt * i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw "More";
        int res = i2 -> get() > i1 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
    void print() const{
        printf("> ");
    }
};

class RPNFunAssign : public RPNFunction
{
public:
    RPNFunAssign() {};
    virtual ~RPNFunAssign(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        //op1 -> print();
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw "Assign1";
        RPNElem * op2 = pop(stack);
        //op2 -> print();
        RPNVarAddr * i2 = dynamic_cast<RPNVarAddr *>(op2);
        if(!i2)
            throw "Assign2";
        RPNElem * op3 = pop(stack);
        RPNInt * i3 = dynamic_cast<RPNInt *>(op3);
        if(!i3)
            throw "Assign3";
        vars -> assignVar(i2 -> get(), i3 -> get(), i1 -> get());
        delete op1;
        delete op2;
        delete op3;
        return 0;
    }
    void print() const{
        printf(":= ");
    }
};

class RPNPrint : public RPNFunction
{
public:
    RPNPrint(){};
    virtual ~RPNPrint(){};
    RPNElem * evaluateFun(RPNItem ** stack, varInfo * vars) const{
        RPNElem * op1 = pop(stack);
        RPNInt * i1 = dynamic_cast<RPNInt *>(op1);
        RPNString * i2 = dynamic_cast<RPNString *>(op1);
        if(!i1 && !i2)
            throw "Print";
        if(i1)
            printf("%d ", i1 -> get());
        else
            printf("%s ", i2 -> get());
        delete op1;
        return 0;
    }
    void print() const{
        printf("?print ");
    }
};

class RPNNoOp : public RPNFunction
{
public:
	RPNNoOp(){}
	virtual ~RPNNoOp(){}
	RPNElem* evaluateFun(RPNItem ** stack, varInfo * vars) const{
    	return 0;
	}
    void print() const{
        printf("NP ");
    }
};

#endif