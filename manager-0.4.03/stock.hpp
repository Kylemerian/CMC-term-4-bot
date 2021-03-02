// +-------------------------------------------------------------------------+
// |                   Manager game server, vers. 0.4.02                     |
// |    Copyright (c) Andrey Vikt. Stolyarov <avst_AT_cs.msu.ru> 2004-2011   |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                     GNU GENERAL PUBLIC LICENSE, v.2                     |
// | as published by Free Software Foundation      (see the file COPYING)    |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#ifndef STOCK_HPP_SENTRY
#define STOCK_HPP_SENTRY

class Stock {
    struct Item {
        void *id;
        int price;  
        int wanted;
        int satisfied;
    };
    Item *items;
    int max_item_count;
    int item_count;
    int current;
public:
    Stock(int n);
    ~Stock();

    void AddBid(void *id, int amount, int price);
    void Run(int total_amount, bool max_first);
    bool GetWinner(void *&id, int &amount, int &price);
private:
    static bool Better(int a1, int a2, bool max_first);
};

#endif
