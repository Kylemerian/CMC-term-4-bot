#ifndef VARINFO_H
#define VARINFO_H

struct varlist {
	const char * name;
	int value;
	int index; 
	struct varlist *next;
};

struct varInfo
{
    varlist * data;
    varInfo(){
        data = NULL;
    };
    ~varInfo(){};
    void addVar(const char * name, int value, int index){
        varlist * tmp = new varlist;
        tmp -> next = data;
        tmp -> value = value;
        tmp -> name = name;
        tmp -> index = index;
        data = tmp;
    }
    void assignVar(const char * name, int value, int index){
        varlist * tmp = data; 
        while (tmp){
            if (!strcmp(tmp -> name, name) && tmp->index == index){
                tmp->value = value;
                return;
            }
            tmp = tmp->next;
        }
        addVar(name, index, value);
    }
    int findVar(const char * name, int index){
        varlist * tmp = data; 
        while (tmp){
            if (!strcmp(tmp -> name, name) && tmp -> index == index){
                return tmp -> value;
            }
            tmp = tmp -> next;
        }
        addVar(name, index, 0);
        return 0;
    }
    void freemem(){
        varlist * tmp = data; 
        while(data){
            tmp = data;
            data = data->next;
            delete tmp;
        }
    }
};

#endif