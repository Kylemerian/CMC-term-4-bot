// +-------------------------------------------------------------------------+
// |              (S)imple (U)nix (E)vents vers. 0.2.71                      |
// | Copyright (c) Andrey Vikt. Stolyarov <crocodil_AT_croco.net> 2003-2008. |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                 GNU LESSER GENERAL PUBLIC LICENSE, v. 2.1               |
// | as published by Free Software Foundation      (see the file LGPL.txt)   |
// |                                                                         |
// | Please visit http://www.croco.net/software/sue for a fresh copy of Sue. |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




/* This simple demo program just sleeps 5 seconds and exits 
   using SUE mechanism of timeout events 
*/
#include <stdio.h>
#include "sue_sel.hpp"

class AbortingTimeout : public SUETimeoutHandler {
    SUEEventSelector *selector;
public:
    AbortingTimeout(SUEEventSelector *a_sel) { selector = a_sel; }
    ~AbortingTimeout() {}
    virtual void TimeoutHandle() { selector->Break(); }
};

int main() 
{
    SUEEventSelector selector;
    AbortingTimeout atm(&selector);
    atm.SetFromNow(5); // 5 seconds
    selector.RegisterTimeoutHandler(&atm);
    printf("Entering the main loop... \n");
    selector.Go();
    printf("Main loop exited\n");
    return 0;
}

