#ifndef RPN_H
#define RPN_H

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
    virtual void evaluate(RPNItem ** stack, RPNItem ** curCmd) const = 0;
protected:
    static void push(RPNItem ** stack, RPNElem * elem);
    static RPNElem * pop(RPNItem ** stack);
};

class RPNConst : public RPNElem
{
public:
    virtual RPNElem * clone() const = 0;
    void evaluate(RPNItem ** stack, RPNItem ** curCmd) const{
        push(stack, clone());
        *curCmd = (*curCmd) -> next;
    }
};

class RPNFunction : public RPNElem
{
public:
    virtual RPNElem * evaluateFun(RPNItem ** stack) const = 0;
    void evaluate(RPNItem ** stack, RPNItem ** curCmd) const{
        RPNElem * res = evaluateFun(stack);
        if(res)
            push(stack, res);
        *curCmd = (*curCmd)->next;
    }
};

class RPNLabel : public RPNConst
{
    RPNItem * value;
public:
    RPNLabel(RPNItem * a = NULL){value = a; }
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
};

class RPNInt : public RPNConst
{
    int value;
public:
    RPNInt(int a){value = a; }
    virtual ~RPNInt(){};
    virtual RPNElem * clone() const{
        return new RPNInt(value);
    }
    int get() const{
        return value;
    }
};

class RPNOpGo : public RPNElem
{
public:
    RPNOpGo(){};
    virtual ~RPNOpGo(){};
    void evaluate(RPNItem ** stack, RPNItem ** curCmd) const{
        RPNElem* op1 = pop(stack);
        RPNLabel * lab = dynamic_cast<RPNLabel *>(op1);
        if(!lab)
            throw op1;
        RPNItem * addr = lab->get();
        *curCmd = addr;
        delete op1;      
    }
};

class RPNOpGoFalse : public RPNElem
{
public:
	RPNOpGoFalse(){}
	virtual ~RPNOpGoFalse(){}
	void evaluate(RPNItem ** stack, RPNItem ** cur_cmd) const{
		RPNElem *op1 = pop(stack);
		RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
		if (!i1)
            throw op1;
		RPNElem *op2 = pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel *>(op2);
		if (!lab)
            throw lab;
		if (!i1->get()){
			RPNItem *addr = lab->get();
			*cur_cmd = addr;
		}
        else
			*cur_cmd = (*cur_cmd)->next;  
		delete op1;
		delete op2;
	}
};

class RPNString : public RPNConst
{
    char * value;
public:
    RPNString(){}
	virtual ~RPNString(){}
	RPNString(char *a){
        value = a;
    }
	virtual RPNElem* clone() const{
        return new RPNString(value);
    }
	char* Get() const {
        return value;
    }
};


class RPNVarAddr : public RPNConst
{

};

class RPNVar : public RPNFunction
{

};

class RPNSell : public RPNFunction
{
    
};

class RPNBuy : public RPNFunction
{
    
};

class RPNEndturn : public RPNFunction
{
    
};

class RPNProd : public RPNFunction
{
    
};

class RPNFunPlus : public RPNFunction
{
public:
    RPNFunPlus() {};
    virtual ~RPNFunPlus(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() + i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunUnoMinus : public RPNFunction{
public:
    RPNFunUnoMinus(){}
    virtual ~RPNFunUnoMinus(){}
    RPNElem* evaluateFun(RPNItem ** stack) const{
        	RPNElem *op1 = pop(stack);
        	RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        	if (!i1)
                throw op1;
        	int res = -(i1 -> get());
       	 	delete op1;
        	return new RPNInt(res);
    }
};

class RPNFunMinus : public RPNFunction
{
public:
    RPNFunMinus() {};
    virtual ~RPNFunMinus(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() - i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunMultiply : public RPNFunction
{
public:
    RPNFunMultiply() {};
    virtual ~RPNFunMultiply(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() * i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunDivision : public RPNFunction
{
public:
    RPNFunDivision() {};
    virtual ~RPNFunDivision(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() / i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunMod : public RPNFunction
{
public:
    RPNFunMod() {};
    virtual ~RPNFunMod(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() % i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunNot : public RPNFunction
{
public:
    RPNFunNot() {};
    virtual ~RPNFunNot(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        int res = !(i1 -> get());
        delete op1;
        return new RPNInt(res);
    }
};

class RPNFunAnd : public RPNFunction
{
public:
    RPNFunAnd() {};
    virtual ~RPNFunAnd(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() && i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunOr : public RPNFunction
{
public:
    RPNFunOr() {};
    virtual ~RPNFunOr(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() || i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunEqual : public RPNFunction
{
public:
    RPNFunEqual() {};
    virtual ~RPNFunEqual(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() == i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunLess : public RPNFunction
{
public:
    RPNFunLess() {};
    virtual ~RPNFunLess(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() < i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunMore : public RPNFunction
{
public:
    RPNFunMore() {};
    virtual ~RPNFunMore(){};
    RPNElem * evaluateFun(RPNItem ** stack) const{
        RPNElem * op1 = pop(stack);
        RPNInt *i1 = dynamic_cast<RPNInt *>(op1);
        if(!i1)
            throw op1;
        RPNElem * op2 = pop(stack);
        RPNInt *i2 = dynamic_cast<RPNInt *>(op2);
        if(!i2)
            throw op2;
        int res = i1 -> get() > i2 -> get();
        delete op1;
        delete op2;
        return new RPNInt(res);
    }
};

class RPNFunAssign : public RPNFunction
{
public:
    RPNFunAssign() {};
    virtual ~RPNFunAssign(){};

};

class RPNNoOp : public RPNElem
{
public:
    RPNNoOp(){};
    virtual ~RPNNoOp(){};
    void evaluate(RPNItem ** stack, RPNItem ** curCmd) const{

    };
};

#endif