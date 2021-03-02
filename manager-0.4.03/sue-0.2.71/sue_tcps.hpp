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




#ifndef SENTRY_SUE_TCPS_HPP
#define SENTRY_SUE_TCPS_HPP

#include "sue_sel.hpp"
#include "sue_inet.hpp"

struct sockaddr_in;

class SUETcpServerSession;

//! Generic TCP protocol server
/*! This class implements a generic multiuser TCP server.
  It is inherited from SUEFdHandler because it has the 
  descriptor for listening socket and handles its events.
  In order to create your own server, do the following:
  - create your own SUETcpServerSession subclass
  - create your own SUETcpServer subclass overriding 
    SpawnSession() method to create and return a new 
    object of YOUR SUETcpServerSession
  - pass the ip-address to bind (as a char* constant), 
    port to listen (as an integer) to the constructor 
  - bring up your Selector (an object of SUEEventSelector class
    you use as a main loop framework)
  - call Up() method of your SUETcpServer passing it the 
    reference to YOUR Selector
  - check if everyting's Ok (the Up() returned true)
  - run the main loop (see the SUEFdSelector class description)
*/ 
class SUETcpServer : private SUEFdHandler {
    //! TCP port to listen 
    int port;
    //! ip address to bind to (in text representation)
    char *ipaddr;
    //! Descriptor of the listening socket
    int mainfd;
    //! Our sessions
    struct SessionsListItem {
        SUETcpServerSession *sess;
        SessionsListItem *next;
    } *sessions;

    struct sockaddr_in *acceptedsockaddr;
    
protected: // For overriden SpawnSession 
    //! The Selector to use
    SUEEventSelector *selector;
private:
    //! Handler of the listening socket descriptor's events
    virtual void FdHandle(bool a_r, bool a_w, bool a_ex);
public:
    //! Constructor
    /*! The constructor. Parameters are:
      \param a_ip - the ip-address in the string representation, 
      which you wish your server to bind to. Use 
      "0.0.0.0" to bind to all addresses of your system.
      \param a_port - the TCP port to listen
      \note constructor just constructs the object. It doesn't 
      create a socket nor does it try to bind it or listen etc.
      See the Up() method.
    */
    SUETcpServer(const char *a_ip, int a_port);
    //! Destructor
    virtual ~SUETcpServer();
    //! Bring the server up
    /*! This method creates a socket to listen calling the socket(2)
      system call. Then it binds the socket to the ip address and port
      given to constructor of the object. After that, it calls 
      listen(2) to listen the socket. In case everything's Ok, 
      the method registers the object with the given Selector object 
      to monitor the listening socket's activity.
      Method returns true if everything's Ok, or false if something went
      wrong.
    */
    bool Up(SUEEventSelector *a_selector);
    //! Shut the server down
    /*! This method removes the object from Selector, closes the 
      listening socket and shuts down all active tcp sessions 
      created by the server. 
    */
    void Down();

    //! Get the ip address of the last accepted session 
    /*! returns the ip address in the HOST byte order */
    unsigned long GetIpOfLastAccepted() const;
    
    //! Session got down, kill its object
    /*! This method is called by the SUETcpServerSession object 
      when the session gets down. The method removes the Session
      object from the list of sessions and destroys the Session
      object calling delete sess;
      \note This method is a part of internal communications.
      Forget it unless you'd like to modify the library itself. 
    */
    void NotifySessionDown(SUETcpServerSession* sess);


protected:
    //! Method to create a custom SUETcpServerSession object.
    /*! This method is called when a new request is accepted.
      It just creates an object of a subclass of SUETcpServerSession
      and returns a pointer to it. 
      Override this method to create and return YOUR SUETcpServerSession
      object. For example,
      \verbatim
      SUETcpServerSession* MyServer::SpawnSession(int fd)
      { return 
          new MySession(fd, timeout, selector, this, "MY GREETING\n");
      }
      \endverbatim
      where selector and timeout are names of protected fields of
      SUETcpServer class.
    */  
    virtual SUETcpServerSession* SpawnSession(int newsessionfd) = 0;
};

//! Generic Tcp Session to be used with SUETcpServer
/*! A child class of SUETcpServerSession is the right place
  to implement your protocol. Create YOUR child class overriding 
  the HandleNewInput and HandleSessionTerminatingEvent methods.
  \note This class is used solely by SUETcpServer, that is, it is 
  created and destroyed by SUETcpServer's (or its child) methods,
  and it is stored by the server so the server takes care about the 
  appropriate cleanup. 
  It exists as long as the corresponding session really exists. 
  \note
  The functions HandleSessionTimeout(), HandleRemoteClosing() and
  HandleReadError() are not reimplemented here so by default they
  shut the session down. If you need to perform some actions right 
  when the session dies, put them into your destructor. 
*/
class SUETcpServerSession : public SUEInetDuplexSession {
    friend class SUETcpServer;
    //! The TCP server we belong to
    SUETcpServer *server;
    
protected: 
    //! Constructor
    /*! 
      \param a_fd is the file descriptor of the accepted session
      \param a_timeout is the value of the timeout (in seconds)
      used to close idle sessions
      \param a_selector is the Selector object (used to deal with 
      the file handler and timeout handler)
      \param a_server is the TcpServer whic created this session
      \param a_greeting is the message to send to the client 
      at the very start of the session.
    */
    SUETcpServerSession(int a_fd, int a_timeout, 
                        SUEEventSelector *a_selector,
                        SUETcpServer *a_server,
                        const char *a_greeting = 0);    
    //! Destructor
    /*! \note Destructor made protected because only SUETcpServer is
      allowed to destroy its sessions 
    */
    virtual ~SUETcpServerSession();

private:
    //! Deletes the object when the session dies
    /*! This function is called by the SUETcpServerSession::Shutdown()
      when the session is terminated for any reason. It then asks
      the server object to delete this session. The method
      TcpServerSessionShutdownHook() is called before this is done.
      \note this function is not to be overriden by the inheritors.
      Override TcpServerSessionShutdownHook() instead if you wish to
      do something before you close down.
    */
    virtual void ShutdownHook();
public:
    virtual void HandleNewInput() = 0; 
    
    //virtual void HandleSessionTimeout();
    //virtual void HandleRemoteClosing();
    //virtual void HandleReadError();

    virtual void TcpServerSessionShutdownHook() {}
};

#endif
