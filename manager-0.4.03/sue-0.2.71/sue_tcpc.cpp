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
#include <errno.h>

#include <stdio.h>

#include "sue_tcpc.hpp"


SUETcpClientSession::SUETcpClientSession(const char *a_ipaddr,
                                         int a_ipport,
                                         int a_timeout,
                                         const char *a_greeting,
                                         const char *a_local_ipaddr,
                                         int a_local_ipport)
   : SUEInetDuplexSession(a_timeout, a_greeting)
{
#if 0
#define xx(x) (x ? x : "<0>")
    printf("**** %s %d %d %s %s %d *****\n",
           xx(a_ipaddr), a_ipport, a_timeout,
           xx(a_greeting), xx(a_local_ipaddr), a_local_ipport);
#endif


    ipaddr = strdup(a_ipaddr);
    ipport = a_ipport;
    connection_in_progress = false;
    local_ipaddr = a_local_ipaddr ? strdup(a_local_ipaddr) : 0;
    local_ipport = a_local_ipport;

    
#if 0
    printf("**** remote: %s:%d local: %s:%d  *****\n",
           ipaddr, ipport, xx(local_ipaddr), local_ipport);
#undef xx
#endif    
  
}

SUETcpClientSession::~SUETcpClientSession()
{
    Shutdown();
    if(ipaddr) free(ipaddr);
    if(local_ipaddr) free(local_ipaddr);
}

bool SUETcpClientSession::Up(SUEEventSelector *a_selector)
{
    return Up(a_selector, local_ipaddr, local_ipport);
}

bool SUETcpClientSession::Up(SUEEventSelector *a_selector, 
                             const char *src_ip, int src_port)
{
    struct sockaddr_in SockAddrIn;
    int lfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(lfd == -1) 
        return false;
    if(src_ip) {
        int sockopt = 1;
        setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (char*)&sockopt, 
                   sizeof(sockopt));
        SockAddrIn.sin_family = AF_INET;
        SockAddrIn.sin_port = htons(src_port);
        SockAddrIn.sin_addr.s_addr = inet_addr(src_ip);
        int rc = bind(lfd, (sockaddr*)&SockAddrIn, sizeof(SockAddrIn));
        if(rc == -1) {
            close(lfd);
            return false;
        }
    }
    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_port = htons(ipport);
    SockAddrIn.sin_addr.s_addr = inet_addr(ipaddr);
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    int rc = connect(lfd, (sockaddr *)&SockAddrIn, sizeof(SockAddrIn));
    if(rc == -1 && errno != EINPROGRESS) {
        close(lfd);
        return false;
    }
    connection_in_progress = true;
    Startup(a_selector, lfd);
    return true;
}

bool SUETcpClientSession::WantWrite() const
{
    return connection_in_progress || SUEGenericDuplexSession::WantWrite();
}

void SUETcpClientSession::FdHandle(bool r, bool w, bool ex)
{
    if(!connection_in_progress) {
        SUEInetDuplexSession::FdHandle(r, w, ex);
    } else if(w) {
        connection_in_progress = false; 
        int sockopt; 
        socklen_t sockoptlen = sizeof(sockopt);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&sockopt, &sockoptlen);
        if(sockopt != 0) {
            HandleConnectionFailure(); 
        }
    }
}

