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
#include <sys/socket.h>
#include <netinet/in.h>

#include "sue_inet.hpp"

   
void SUEInetDuplexSession::Startup(SUEEventSelector *a_selector, 
                                   int a_fd, const char *a_greeting)
{
    SUEGenericDuplexSession::Startup(a_selector, a_fd, a_greeting); 
    struct sockaddr_in s_in;
    socklen_t s_len = sizeof(s_in);
    int res = getsockname(fd, (struct sockaddr *)(&s_in), &s_len);
    if(res < 0) {
        throw SUEException("getsockname failed in InetSession Startup");
    }
    saved_local_ip = ntohl(s_in.sin_addr.s_addr);
}
