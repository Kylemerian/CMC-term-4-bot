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




#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>


#include "sue_wait.hpp"

SUEChildHandler::SUEChildHandler(pid_t a_pid, SUEChildWaitAgent *a_agent)
{
    status = 0;
    pid = a_pid;
    a_agent->RegisterChildHandler(this);
}

bool SUEChildHandler::IfExited() const
{
    return WIFEXITED(status);
}

int SUEChildHandler::ExitCode() const
{
    return WEXITSTATUS(status);
}

bool SUEChildHandler::IfSignaled() const
{
    return WIFSIGNALED(status);
}

int SUEChildHandler::TermSig() const
{
    return WTERMSIG(status);
}



SUEChildWaitAgent::SUEChildWaitAgent()
    : SUESignalHandler(SIGCHLD)
{
    the_selector = 0;
    first = 0;
}
    
    
SUEChildWaitAgent::~SUEChildWaitAgent()
{
    Unregister();
    while(first) {
        ChildListItem *tmp = first->next;
        delete first;
        first = tmp;
    }
}

void SUEChildWaitAgent::Register(SUEEventSelector *a_selector)
{
    the_selector = a_selector;
    the_selector->RegisterSignalHandler(this);
}

void SUEChildWaitAgent::Unregister()
{
    if(the_selector) the_selector->RemoveSignalHandler(this);
    the_selector = 0;
}


void SUEChildWaitAgent::RegisterChildHandler(SUEChildHandler *h)
{
    ChildListItem *tmp = new ChildListItem;
    tmp->handler = h;
    tmp->next = first;
    first = tmp;
}

void SUEChildWaitAgent::RemoveChildHandler(SUEChildHandler *h)
{
    ChildListItem **tmp = &first;  
    while(*tmp && (*tmp)->handler != h) {
      tmp = &((*tmp)->next);
    }
    if(*tmp) {
      ChildListItem *tmp2 = *tmp;
      *tmp = (*tmp)->next; 
      delete tmp2;
    }
}

#if 0
#include <stdio.h>
static void print_that_list(SUEChildWaitAgent::ChildListItem *first)
{
    SUEChildWaitAgent::ChildListItem *tmp = first;
    printf("* ");
    while(tmp) {
        pid_t pid = tmp->handler->GetPid();
        printf("%d ", pid);
        tmp = tmp->next;
    }
    printf("\n");

}
#endif

void SUEChildWaitAgent::SignalHandle()
{
    ChildListItem **tmp = &first;
    while(*tmp) {
        pid_t pid = (*tmp)->handler->GetPid();
        int status;
        int rc = waitpid(pid, &status, WNOHANG);
        if(rc == -1 && errno == ECHILD)
            throw SUEException("Invalid child pid");
        if(rc == pid) {
            ChildListItem *tmp2 = *tmp;
            tmp2->handler->AgentsHandler(status);
            *tmp = (*tmp)->next;
            delete tmp2;
        } else {
            tmp = &((*tmp)->next); 
        }
    }
}

