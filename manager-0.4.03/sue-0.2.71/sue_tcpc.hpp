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




#ifndef UPP_TCPC_HPP_SENTRY
#define UPP_TCPC_HPP_SENTRY

#include "sue_inet.hpp"

//! Client TCP session
/*! This class deals with a client TCP session. You create your own
    class to represent whatever protocol you need. It is done in 
    methods HandleNewInput() and HandleSessionTerminatingEvent() (see 
    the UPPGenericDuplexSession class description for details on 
    implementing a session object).
    \par 
    Having the class, you first create an object as appropriate, and 
    then start your session by calling Up() method. Your handling methods
    may instruct other objects to write something to the session whenever
    they need (that is done using WriteChars() method). The session may be
    shut down any time using Down() method (the preferred way). If you need
    to notify someone the session's dead, override the ShutdownHook()
    method. In case you do, be sure to call Down() or Shutdown() from your
    destructor, or else the session will shut down during destruction of 
    the base class so wrong version of ShutdownHook() is called. Yes, it
    IS safe to call Down() for already-shuted session. ShutdownHook()
    is called only once, when the session is actually shot.   
 */
class SUETcpClientSession : public SUEInetDuplexSession {
    //! The ip address to connect to
    char *ipaddr;
    //! The port to connect to
    int ipport;
    //! The ip address to connect FROM
    char *local_ipaddr;
    //! The port to connect FROM
    int local_ipport;
    
    //! 'connection in progress' flag
    bool connection_in_progress; 
public:
    const char *InspectAddr() const { return ipaddr; }
    int InspectPort() const { return ipport; }
    const char *InspectLocalAddr() const { return local_ipaddr; }
    int InspectLocalPort() const { return local_ipport; }

    //! Constructor
    /*! \param ipaddr - the ip-address to connect to (in the text
               representation)
        \param ipport - the TCP port to connect to.
        \param timeout - the timeout in seconds. If a session idles 
               more than than many seconds, it dies. 
        \param greeting - the string to send via the socket right after
               connection is estableshed, without waiting for any data from
               the other end. If unneeded, NULL may be passed.
        \param local_ip - the local ip to use (or NULL to let the
               system choose)
        \param localport - the local TCP port (or 0 to let the system choose)
        \note constructor just constructs the object. It doesn't 
              create a socket nor does it try to connect. 
              See the Up() method.
        \note Up() will call bind() in case the local_ip or local_port
              or both are specified. Otherwise, bind() isn't used. 

     */
    SUETcpClientSession(const char *ipaddr, int ipport,
                        int timeout, const char *greeting = 0,
                        const char *local_ip = 0, int localport = 0);
    //! Destructor
    virtual ~SUETcpClientSession();

    //! Actually start the session
    /*! The method calls socket(2), then tries to connect(2) to
        the desired destination. In case of success, registers the
        session and the timeout at the selector and returns true.
        In case socket(2) or connect(2) fails, immediately returns 
        false
     */  
    bool Up(SUEEventSelector *a_selector);

    //! Start the session performing connect from the given address
    /*! This version of the method calls socket(2), then bind(2),
        then connect(2), so that the connection is made from the given
        address.
    */
    bool Up(SUEEventSelector *a_selector,
            const char *src_ipaddr, int src_port);

    
    //! Shuts the session down
    /*! The preferred method to shut the session down. 
     */
    void Down() { Shutdown(); }

    //! What to do if connection attempt failed
    /*! This function is called when, after connect(2) had been called,
        select(2) indicated writability while getsockopt(2) indicated
        the connection wasn't successfull. By other words, the session 
        is considered 'up' (because Up() returned true), but after that, 
        it couldn't connect to the destination.
        \par
        By default, the function calls Shutdown() just like all the 
        similar methods like HandleSessionTimeout() etc, but you can override 
        it to customize this.
     */
    virtual void HandleConnectionFailure() { Shutdown(); }

    virtual void HandleNewInput() = 0; 
    
    //virtual void HandleSessionTimeout();
    //virtual void HandleRemoteClosing();
    //virtual void HandleReadError();

    //virtual void ShutdownHook();

private:
    virtual bool WantWrite() const;
    virtual void FdHandle(bool a_r, bool a_w, bool a_ex); 
};


#endif
