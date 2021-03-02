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




#ifndef SENTRY_SUE_INET_HPP
#define SENTRY_SUE_INET_HPP

#include "sue_sess.hpp"


//! Duplex session based on AF_INET sockets
/*!
  This class is intended to incapsulate the functionality common for 
  AF_INET sockets.
  \par
  This particular verision adds to a generic duplex session object the
  capability of knowing the ip address of the local end of the connection.
*/
class SUEInetDuplexSession : public SUEGenericDuplexSession {
      //! result of getsockname(2)
    unsigned long saved_local_ip;
public:
    SUEInetDuplexSession(int a_timeout,
                         const char *a_greeting = 0)
        : SUEGenericDuplexSession(a_timeout,
                                  a_greeting) 
        { saved_local_ip = 0xffffffff; }
    virtual ~SUEInetDuplexSession() {}

      //! Start the session
      /*! This method hides (incapsulates) the
          SUEGenericDuplexSession::Startup() method in order to do some
          specific manipulations.
          \par In this particular version the only manipulation is to
          get and store the ip address of the local end of the
          connection.
       */
    void Startup(SUEEventSelector *a_selector,
                 int a_fd, const char *a_greeting = 0);
    
      //! IP of the local endpoint
      /*!
        The method returns the ip-address of the local endpoint
        of the connected socket (obtained with getsockname(2)).
        It is perfectly safe to request the address even when the
        session is already shot down because the address is always
        requested when the session starts up.
        \note The address is returned in the HOST BYTE ORDER.
      */
    unsigned long GetLocalIp() const { return saved_local_ip; }
};



#endif
