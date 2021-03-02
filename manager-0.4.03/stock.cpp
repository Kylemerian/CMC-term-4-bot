// +-------------------------------------------------------------------------+
// |                   Manager game server, vers. 0.4.01                     |
// |    Copyright (c) Andrey Vikt. Stolyarov <avst_AT_cs.msu.ru> 2004-2006   |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                     GNU GENERAL PUBLIC LICENSE, v.2                     |
// | as published by Free Software Foundation      (see the file COPYING)    |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#include <stdlib.h>
#include "stock.hpp"



Stock::Stock(int n)
{
    max_item_count = n;
    item_count = 0;
    items = new Item[n];
    current = 0;
}

Stock::~Stock()
{
    delete [] items;
}


void Stock::AddBid(void *id, int amount, int price)
{
    if(item_count>=max_item_count) 
        throw "BUG: Stock::AddBid: too many bids";
    items[item_count].id = id;
    items[item_count].price = price;
    items[item_count].wanted = amount;
    items[item_count].satisfied = 0;
    item_count++;
}

void Stock::Run(int total_amount, bool max_first)
{
    while(total_amount > 0) {
        int i=0;
        // find the first unsatisfied
        while(i<item_count && items[i].wanted==0) i++;      
        if(i>=item_count) return; // everyone's satisfied
        int best_price = items[i].price;
        int best_count = 1;
        i++;
        for(; i<item_count; i++) {
            if(items[i].wanted<1) continue;
            if(Better(items[i].price, best_price, max_first)) {
                best_price = items[i].price;
                best_count = 1;
            } else if(items[i].price == best_price) {
                best_count++;
            }
        }
        if(best_count<0 || best_count>item_count)
            throw "BUG: Stock::Run: strange value";
        int iteration_winner = 
            best_count == 1 ? 1 : 
                1+(int)((best_count*1.0)*rand()/(RAND_MAX+1.0));
        for(i=0; i<item_count; i++) {
            if(items[i].wanted<1) continue;
            if(items[i].price != best_price) continue;
            if(iteration_winner > 1) {
                iteration_winner--;
                continue;
            }
            // NOW!!!
            int sum = items[i].wanted > total_amount ? 
                             total_amount : items[i].wanted;
            items[i].wanted -= sum;  
            items[i].satisfied += sum;
            total_amount -= sum;
            break;  
        }
    }
}

bool Stock::GetWinner(void *&id, int &amount, int &price)
{
    while(current<item_count && items[current].satisfied==0) current++;
    if(current>=item_count) return false;
    id = items[current].id;
    amount = items[current].satisfied;
    price = items[current].price;
    current++;
    return true;
}

bool Stock::Better(int a1, int a2, bool max_first)
{
    return max_first ? a1 > a2 : a1 < a2;
}
