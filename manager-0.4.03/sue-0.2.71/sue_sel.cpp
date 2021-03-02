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




#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sue_sel.hpp"


// this class is used internally by the library
class SUESignalQueue {

#   ifndef SUE_MAX_SIGNAL_QUEUE_SIZE
#   define SUE_MAX_SIGNAL_QUEUE_SIZE 1024
#   endif

    sig_atomic_t queue[SUE_MAX_SIGNAL_QUEUE_SIZE];
    sig_atomic_t que_p1, que_p2;

    static void TheHandlingFunction(int signo);
    
    sigset_t current;
    
public:
    SUESignalQueue() {
        que_p1 = que_p2 = 0; 
        sigemptyset(&current);
    }
    ~SUESignalQueue() {
        RemoveAll();
    }
    
    void Add(int signo) { 
        struct sigaction sa;
        sa.sa_handler = TheHandlingFunction;
        sigfillset(&(sa.sa_mask));
        sa.sa_flags = 0;
        sigaction(signo, &sa, 0);
        sigaddset(&current, signo); 
    }

    void Remove(int signo) { 
        struct sigaction sa;
        sigdelset(&current, signo); 
        sa.sa_handler = SIG_DFL;
        sigfillset(&(sa.sa_mask));
        sa.sa_flags = 0;
        sigaction(signo, &sa, 0);
    }

    void RemoveAll() { 
        for(unsigned int i=1; i<=(sizeof(current)+7)/8; i++)
            if(sigismember(&current, i)) Remove(i);
    }

    bool FetchSignalEvent(int &signo) {
        if(que_p1==que_p2) return false;
        signo = queue[que_p1];
        que_p1++;
        if(que_p1>=SUE_MAX_SIGNAL_QUEUE_SIZE) que_p1=0;
        return true;
    }
    
private:
    void AddEvent(int signo) {
        int i = que_p2++;
        if(que_p2>=SUE_MAX_SIGNAL_QUEUE_SIZE) que_p1=0;
        if(que_p2==que_p1) { // no more space in the queue
            // just refuse to store the signal
            que_p2 = i;
            return; 
        } 
        queue[i]=signo;
    }
};

static SUESignalQueue TheSignalQueue;

void SUESignalQueue::TheHandlingFunction(int signo)
{
    TheSignalQueue.AddEvent(signo);
}



// SUEFdHandler

SUEFdHandler::SUEFdHandler(int a_fd)
{
    fd = a_fd; 
}

SUEFdHandler::~SUEFdHandler()
{ /* Nothing to be done here... really? */ }


// SUETimeoutHandler

SUETimeoutHandler::SUETimeoutHandler(long a_sec, long a_usec)
{
    sec = a_sec; usec = a_usec; 
}

SUETimeoutHandler::SUETimeoutHandler()
{
    sec = -1; usec = -1; 
}

SUETimeoutHandler::~SUETimeoutHandler()
{ /* Nothing to be done here... really? */ }

void SUETimeoutHandler::Set(long a_sec, long a_usec)
{
    sec = a_sec; usec = a_usec; 
}

void SUETimeoutHandler::SetFromNow(long a_sec, long a_usec)
{
    struct timeval current;
    gettimeofday(&current, 0 /* timezone unused */);
    sec = current.tv_sec + a_sec; 
    usec = current.tv_usec + a_usec;
    if(usec >= 1000000) {
        sec += usec / 1000000;
        usec = usec % 1000000;
    }
}

void SUETimeoutHandler::Get(long &a_sec, long &a_usec) const
{
    a_sec = sec; 
    a_usec = usec; 
}

void SUETimeoutHandler::GetRemainingTime(long &a_sec, long &a_usec) const
{
    struct timeval current;
    gettimeofday(&current, 0 /* timezone unused */);
    a_sec = sec - current.tv_sec;
    a_usec = usec - current.tv_usec;
    if(a_usec < 0) {
        a_usec += 1000000;
        a_sec -= 1;
    }
}


// SUESelector

SUEEventSelector::SUEEventSelector()
{
    timeouts = 0;
    signalhandlers = 0;
    loophooks = 0;
    fdhandlerssize = 10; // It's unlikely that there will be more FDs
    fdhandlers = new SUEFdHandler* [fdhandlerssize];
    int i; for (i=0; i<fdhandlerssize; i++) fdhandlers[i]=0;
}

SUEEventSelector::~SUEEventSelector()
{
    while(timeouts) {
        TimeoutsListItem *tmp = timeouts->next;
        delete timeouts;
        timeouts = tmp;
    } 
    delete [] fdhandlers;
}

void SUEEventSelector::RegisterFdHandler(SUEFdHandler *h)
{
    if(h->fd >= fdhandlerssize) 
        ResizeFdHandlers(h->fd+1);
    if(fdhandlers[h->fd]) // error, duplicate handler
        throw SUEException("duplicate fd handler");
    fdhandlers[h->fd] = h;
}

void SUEEventSelector::RemoveFdHandler(SUEFdHandler *h) 
{
    // please note we shouldn't use the value of f->fd as
    // the array's index because it can be invalid for some reasons
    for(int i=0; i<fdhandlerssize; i++) {
        if(fdhandlers[i] == h) {
            fdhandlers[i] = 0;
            return;
        }
    }
}

void SUEEventSelector::RegisterTimeoutHandler(SUETimeoutHandler *h)
{
    TimeoutsListItem *tmp = new TimeoutsListItem;
    tmp->handler = h;
    if(!timeouts) {
        tmp->next = 0;
        timeouts = tmp;  
    } else {
        TimeoutsListItem **tmp2 = &timeouts;  
        while (*tmp2 && *((*tmp2)->handler) < *h) {
            tmp2 = &((*tmp2)->next);
        }
        if(!(*tmp2)) {
            *tmp2 = tmp;
            tmp->next = 0;
        } else {
            if((*tmp2)->handler == h) { // it is the SAME object
              delete tmp;
              return; 
            }
            tmp->next = *tmp2;
            *tmp2 = tmp;
        }
    }
}

void SUEEventSelector::RemoveTimeoutHandler(SUETimeoutHandler *h)
{
    TimeoutsListItem **tmp = &timeouts;  
    while(*tmp && (*tmp)->handler != h) {
        tmp = &((*tmp)->next);
    }
    if(*tmp) {
        TimeoutsListItem *tmp2 = *tmp;
        *tmp = (*tmp)->next; 
        delete tmp2;
    }
}

void SUEEventSelector::RegisterSignalHandler(SUESignalHandler *h)
{
    SignalListItem *tmp = new SignalListItem;
    tmp->handler = h;
    tmp->next = signalhandlers;
    signalhandlers = tmp;
    TheSignalQueue.Add(tmp->handler->signo);
}

void SUEEventSelector::RemoveSignalHandler(SUESignalHandler *h)
{
    int signo = h->signo;
    SignalListItem **tmp = &signalhandlers;  
    while(*tmp && (*tmp)->handler != h) {
        tmp = &((*tmp)->next);
    }
    if(*tmp) {
        SignalListItem *tmp2 = *tmp;
        *tmp = (*tmp)->next; 
        delete tmp2;
    }  
    for(SignalListItem *p = signalhandlers; p; p=p->next)
        if(p->handler->signo==signo) return; // another one exists
    // else, remove it
    TheSignalQueue.Remove(signo);
}

void SUEEventSelector::RegisterLoopHook(SUELoopHook *h)
{
    LoopHookListItem *tmp = new LoopHookListItem;
    tmp->next = loophooks;
    tmp->hook = h;
    loophooks = tmp;
}

void SUEEventSelector::RemoveLoopHook(SUELoopHook *h)
{
    for(LoopHookListItem **p = &loophooks; *p; p = &((*p)->next)) {
        if((*p)->hook == h) {
            LoopHookListItem *to_del = *p;
            *p = (*p)->next;
            delete to_del;
            return;
        }
    }
}
 
struct timeval* SUEEventSelector::ComputeClosestTimeout(struct timeval &timeout)
{
    struct timeval current;
    int rc = gettimeofday(&current, 0 /* timezone unused */);
    if(rc != 0) 
        throw SUEException("gettimeofday failed");
    if(timeouts) {
        timeout.tv_sec = timeouts->handler->sec - current.tv_sec;
        timeout.tv_usec = timeouts->handler->usec - current.tv_usec;
        while(timeout.tv_usec < 0) {
            timeout.tv_usec += 1000000;
            timeout.tv_sec -= 1;
        }
        if(timeout.tv_sec < 0) {
            // This means it's time to handle the timeout
            // Note this might be handled another way, _before_ the select call
            // I preferred to handle it _after_ because it is possible 
            // data we waited for are already arrived which we can determine 
            // only after the call to select; may be the timeout handler will 
            // disappear...
            timeout.tv_sec = 0; timeout.tv_usec = 0;
        }
        return &timeout;
    } else {
        return 0;
    }   
}

struct SelectDescriptorsSet {
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
};

void SUEEventSelector::SetupFdSets(SelectDescriptorsSet &d)
{
    FD_ZERO(&d.readfds);
    FD_ZERO(&d.writefds);
    FD_ZERO(&d.exceptfds);
    for(int i=0; i<fdhandlerssize; i++) {
        if(fdhandlers[i]) {
            if(fdhandlers[i]->WantRead()) FD_SET(i, &d.readfds);
            if(fdhandlers[i]->WantWrite()) FD_SET(i, &d.writefds);
            if(fdhandlers[i]->WantExcept()) FD_SET(i, &d.exceptfds);
        }
    }
}

void SUEEventSelector::HandleSignals() 
{
    int signo;
    while(TheSignalQueue.FetchSignalEvent(signo)) {
        SignalListItem *sig = signalhandlers;
        while(sig) {
            if(sig->handler->signo == signo) 
                sig->handler->SignalHandle();
            sig = sig->next;
        }
    }
}

void SUEEventSelector::HandleLoopHooks() 
{
    for(LoopHookListItem *tmp = loophooks; tmp; tmp = tmp->next)
        tmp->hook->LoopHook();
}

void SUEEventSelector::HandleFds(SelectDescriptorsSet &d)

{
    for(int i=0; i<fdhandlerssize; i++) {
        if(fdhandlers[i]&&(FD_ISSET(i, &d.readfds)||
                           FD_ISSET(i, &d.writefds)||
                           FD_ISSET(i, &d.exceptfds)
                          )
        ) {
             fdhandlers[i]->FdHandle(FD_ISSET(i, &d.readfds),  
                                     FD_ISSET(i, &d.writefds),
                                     FD_ISSET(i, &d.exceptfds));
        }
    } 
}

void SUEEventSelector::HandleTimeouts(struct timeval &current) 
{
    while(timeouts &&
          timeouts->handler->IsBefore(current.tv_sec, current.tv_usec))
    {
        SUETimeoutHandler *hdl = timeouts->handler;
        TimeoutsListItem *tmp = timeouts;
        timeouts = timeouts->next;
        delete tmp;
        hdl->TimeoutHandle(); // In some cases this can delete hdl ! 
    }
}

void SUEEventSelector::Go()  
{
    breakflag = false;
    do {
        SelectDescriptorsSet d;    
        SetupFdSets(d);
        struct timeval timeout;
        struct timeval *pt = ComputeClosestTimeout(timeout); 
        /////////////////////////////////////////////////////////////////////
        int rc = 
        select(fdhandlerssize, &d.readfds, &d.writefds, &d.exceptfds, pt);
        /////////////////////////////////////////////////////////////////////
        if(rc<0 && errno!=EINTR) {
            HandleSelectFailure(rc);
        }
        // Remember the time select(2) returned
        // we can't do it before that if because it could change errno
        struct timeval current;
        gettimeofday(&current, 0 /* timezone unused */);
        HandleSignals();
        if(rc>0) { // file descriptors changed status
            HandleFds(d);
        }
        // Now handle the timeouts as of the select's return moment
        HandleTimeouts(current);
        // The last thing to do is call all main loop hooks
        HandleLoopHooks();
    } while(!breakflag);
}

void SUEEventSelector::Break()
{
    breakflag = true;  
}

void SUEEventSelector::ResizeFdHandlers(int newsize)
{
    int oldsize = fdhandlerssize;
    SUEFdHandler** oldhandlers = fdhandlers;
    if(newsize <= fdhandlerssize) { // can't shrink!
        throw SUEException("attempt to shrink fd array");
    }
    fdhandlers = new SUEFdHandler* [fdhandlerssize = newsize];
    int i;
    for(i=0; i<oldsize; i++) 
        fdhandlers[i] = oldhandlers[i];
    for(; i<newsize; i++)
        fdhandlers[i] = 0;
    delete [] oldhandlers;
}




