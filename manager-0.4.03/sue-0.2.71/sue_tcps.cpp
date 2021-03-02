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




#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "sue_tcps.hpp"


SUETcpServer::SUETcpServer(const char *a_ip, int a_port)
{
    port = a_port;
    ipaddr = strdup(a_ip);
    selector = 0;
    mainfd = -1;
    sessions = 0;
    acceptedsockaddr = new sockaddr_in;
}

SUETcpServer::~SUETcpServer()
{
    if(ipaddr) free(ipaddr);
    while(sessions) {
        SessionsListItem *tmp = sessions;
        sessions = tmp->next;
        delete tmp->sess;
        delete tmp;
    }
    delete acceptedsockaddr;
    if(mainfd != -1) {
        shutdown(mainfd, 2);
        close(mainfd);
    }
}

bool SUETcpServer::Up(SUEEventSelector *a_selector)
{
    selector = a_selector;
    struct sockaddr_in SockAddrIn;
    mainfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    int sockopt = 1;
    setsockopt(mainfd, SOL_SOCKET, SO_REUSEADDR, (char*)&sockopt, 
                                                 sizeof(sockopt));
    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_port = htons(port);
    SockAddrIn.sin_addr.s_addr = inet_addr(ipaddr);
    if (0!=bind(mainfd, (struct sockaddr*)&SockAddrIn, sizeof(SockAddrIn))) {
        /* Error binding the socket */
        return false;
    }
    if (0 != listen(mainfd, 5)) {
        /* Error listening port */
        return false;
    }
    SetFd(mainfd);
    selector->RegisterFdHandler(this);
    return true;
}

void SUETcpServer::Down()
{
    selector->RemoveFdHandler(this); 
    close(mainfd);
    while(sessions) sessions->sess->Shutdown();
}

void SUETcpServer::FdHandle(bool a_r, bool /*a_w*/, bool /*a_ex*/)
{
    if(!a_r) return;  /* this must be a bug, but... let it be ;-) */
    socklen_t SockAddrLen;
    SockAddrLen = sizeof(sockaddr_in);
    int conn = accept(mainfd, (sockaddr*)acceptedsockaddr, &SockAddrLen);
    fcntl(conn, F_SETFL, O_NONBLOCK);
    SessionsListItem *tmp = new SessionsListItem;
    tmp->sess = SpawnSession(conn);
    tmp->next = sessions;
    sessions = tmp;
}

unsigned long SUETcpServer::GetIpOfLastAccepted() const
{
    return ntohl(acceptedsockaddr->sin_addr.s_addr);
}

void SUETcpServer::NotifySessionDown(SUETcpServerSession *sess)
{
    SessionsListItem **tmp = &sessions;  
    while(*tmp && (*tmp)->sess != sess) {
       tmp = &((*tmp)->next);
    }
    if(*tmp) {
        SessionsListItem *tmp2 = *tmp;
        *tmp = (*tmp)->next; 
        delete tmp2->sess;
        delete tmp2;
    } else {
        // Removing non-own session - BUG
        throw SUEException("Cant remove foreign TCP session");
    }
}



SUETcpServerSession::SUETcpServerSession(int a_fd, int a_timeout, 
                                     SUEEventSelector *a_selector,
                                     SUETcpServer *a_server,
                                     const char *a_greeting)    
   : SUEInetDuplexSession(a_timeout, a_greeting)
{
    server = a_server;
    Startup(a_selector, a_fd);
}

SUETcpServerSession::~SUETcpServerSession()
{}

void SUETcpServerSession::ShutdownHook()
{
    TcpServerSessionShutdownHook();
    server->NotifySessionDown(this);
}
