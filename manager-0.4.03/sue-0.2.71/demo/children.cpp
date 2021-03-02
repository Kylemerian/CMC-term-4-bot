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




/* This simple demo program launches a child once a second and looks
   after existing children using the SUE wait support. 
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sue_sel.hpp"
#include "sue_wait.hpp"


class ChildHandler : public SUEChildHandler {
public:
    ChildHandler(pid_t a_pid, SUEChildWaitAgent *a_agent)
        : SUEChildHandler(a_pid, a_agent) {}

    virtual void ChildHandle() {
        printf("Process %d finished (%s), status = %d\n",
               GetPid(), IfExited() ? "exited" : "signaled", ExitCode());
    }
};

void Launch(SUEChildWaitAgent *agent)
{
    int r; 
    int secs = rand() % 10 + 2;
    int ecode = rand() % 50;
    r = fork();
    if(r == -1) {
        perror("fork");
        return;
    }
    if(r == 0) {
        /* child */
        sleep(secs);
        exit(ecode);
    }
    /* parent */
    printf("Launched process %d\n", r);
    new ChildHandler(r, agent);
}

class LaunchingTimeout : public SUETimeoutHandler {
    SUEEventSelector *the_selector;
    SUEChildWaitAgent *the_agent; 
       // got to remember the agent to call the Launch() function
public:
    LaunchingTimeout(SUEEventSelector *a_sel,
                     SUEChildWaitAgent *a_ag) 
    { 
        the_selector = a_sel; 
        the_agent = a_ag;
    }
    ~LaunchingTimeout() {}
    virtual void TimeoutHandle() { 
        Launch(the_agent);
        SetFromNow(1);
        the_selector->RegisterTimeoutHandler(this); 
    }
};

int main() 
{
    SUEEventSelector selector;
    SUEChildWaitAgent agent;

    LaunchingTimeout ltm(&selector, &agent);

    agent.Register(&selector);
    ltm.SetFromNow(0); // start right now
    selector.RegisterTimeoutHandler(&ltm);

    printf("Entering the main loop... \n");
    selector.Go();
    printf("Main loop exited\n");

    return 0;
}

