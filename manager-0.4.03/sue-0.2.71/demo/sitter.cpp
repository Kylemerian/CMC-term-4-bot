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




/* 
  This program listens to you no matter what bullshit you tell, 
  until you get tired and sleepy. If you keep silence for 10 seconds,
  the program assumes you went to sleep and it is no longer needed, 
  so it terminates.
*/
#include <stdio.h>
#include <unistd.h>
#include "sue_sel.hpp"

class PerfectListener : private SUEFdHandler,
                        private SUETimeoutHandler
{
    SUEEventSelector *selector;
    virtual bool WantRead() const { return true; }
    virtual bool WantWrite() const { return false; }
    virtual void TimeoutHandle() 
      { 
          selector->Break(); 
          printf("Sleep, baby, sleep...\n");
      }
    virtual void FdHandle(bool, bool, bool) 
      { 
          char buf[4096]; 
          if(read(0, buf, sizeof(buf))<=0) {
              printf("Error reading stdin or end of file\n");
              selector->Break();
              return;
          } 
          selector->RemoveTimeoutHandler(this);
          SetFromNow(10);
          selector->RegisterTimeoutHandler(this);
      }
public:
    PerfectListener(SUEEventSelector *sel) 
      {
          selector = sel;
          SetFd(0); // stdin
          SetFromNow(10);
          selector->RegisterFdHandler(this);
          selector->RegisterTimeoutHandler(this);
      }
    ~PerfectListener() {} 
};

int main() 
{
    SUEEventSelector selector;
    PerfectListener  pfl(&selector);
    printf("Entering the main loop... \n");
    selector.Go();
    printf("Main loop exited\n");
    return 0;
}

