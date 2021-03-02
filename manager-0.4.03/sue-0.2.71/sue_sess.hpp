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




#ifndef SENTRY_SUE_SESS_HPP
#define SENTRY_SUE_SESS_HPP


#ifndef SENTRY_SUE_SEL_HPP
#include "sue_sel.hpp"
#endif


//! Buffer used by sessions
class SUEBuffer {
    char *data;
    int datalen;
    int maxlen;
public:
      //! Constructor
    SUEBuffer();
      //! Destructor
    ~SUEBuffer();
    
      //! Add some data to the end 
      /*! The buffer is enlarged and data is added */
    void AddData(const char *buf, int size);
    
      //! Move data from the buffer
      /*! The first bufsize bytes from the buffer are
          copied to the user-supplied memory pointed by buf. 
          If the object contains less data than bufsize, then 
          all the present data is copied. The method returns 
          count of the copied bytes. The copied data is removed
          from the buffer.
       */
    int GetData(char *buf, int bufsize);

      //! Drop some data
      /*! The first len bytes in the buffer are removed. If the
          buffer contains less than len bytes, it is just emptied.
       */
    void DropData(int len);

      //! Erase given range from the buffer
    void EraseData(int index, int len);

      //! Empty the buffer
    void DropAll() { datalen = 0; } 
    
      //! Add a char to the end of the buffer
    void AddChar(char c);

      //! Add a string to the end of the buffer
      /*! The string pointed by str is added to the buffer.
          Terminating zero is not copied, only the string itself.
       */
    void AddString(const char *str);

      //! Read a line
      /*! Checks whether a text line (that is, something terminated 
          by the EOL character) is available at the beginning of the
          buffer. If so, copy it to the user-supplied memory and remove
          it from the buffer. 
          \note The zero byte is always stored into the caller's buffer,
          so at most bufsize-1 bytes are copied. If there is no room 
          for the whole string, than only a part is copied. 
          The terminating EOL is never stored in the buffer. 
       */
    int ReadLine(char *buf, int bufsize);
      //! Read a line into another buffer
      /*! Checks whether a text line (that is, something terminated 
          by the EOL character) is available at the beginning of the
          buffer. If so, copy it to the user-supplied buffer and remove
          it from the buffer. 
          \note The zero byte is always stored into the caller's buffer,
          so at most bufsize-1 bytes are copied. If there is no room 
          for the whole string, than only a part is copied. 
          The terminating EOL is never stored in the buffer. 
       */
    bool ReadLine(SUEBuffer &buf);
    
      //! Find the given line in the buffer
      /*! returns the index of the '\n' right after the marker */ 
    int FindLineMarker(const char *marker) const;
    int ReadUntilLineMarker(const char *marker, char *buf, int bufsize);
    bool ReadUntilLineMarker(const char *marker, SUEBuffer &dest);
    
      //! Does the buffer contain exactly given text
      /*! Checks if the buffer's content is exactly the same
          as in the given zero-terminated string.  
       */
    bool ContainsExactText(const char *str) const;
    
      //! Return the pointer to the actual data
      /*! \note only the first len bytes makes sence,
          where len is what Length() method returns. 
          Accessing addresses beyond this amount could
          lead to an unpredictable behaviour and crash 
          the program!
       */
    const char *GetBuffer() const { return data; }
      //! How much data is in the buffer
      /*! How many bytes does the buffer contain */
    int Length() const { return datalen; }
      //! Access the given byte
      /*! \warning No range checking is performed. Passing
          negative i or i more than the current buffer length
          (as returned by Length() method) could lead to an 
          unpredictable behaviour and/or crash.
       */
    char& operator[](int i) const { return *(data + i); }

private:
    void ProvideMaxLen(int n);
};


//! Duplex session via a single file descriptor
/*! This class provides an abstract hope-to-be-generic
    duplex connection via a single file descriptor
    (e.g., a duplex socket). 
    The class is capable of timing out the session. 
    \par 
    All the input/output is made asynchronously and is hidden
    from the user. You just operate with two protected fields 
    named inputbuffer and outputbuffer. They are both of type 
    SUEBuffer. Chars read from the file descriptor are appended 
    to the end of the inputbuffer. Chars in the outputbuffer 
    are considered as chars to be sent to through the file 
    descriptor. 
    \par
    You must create a child class of this and the child must
    override at least the HandleNewInput() and HandleRemoteClosing()
    methods. You might want to override ShutdownHook(), 
    HandleSessionTimeout() and/or HandleReadError() as well.
    \par
    Use Startup(), Shutdown() and GracefulShutdown() to control
    the session.
    \note You can make your objects delete themselves once the session
    is terminated. In this case, override ShutdownHook() and put the 
    operator delete this; there. 
    \note The other option is to close the session by deleting the
    object. It is possible because Shutdown() is always called from
    the destructor. But be sure you do NOT delete the object from
    within ShutdownHook() in this case!
    \warning And once again, either you delete the object manually,
    or you let the object delete itself from ShutdownHook(), but 
    not both.
 */
class SUEGenericDuplexSession : protected SUEFdHandler, 
                                private SUETimeoutHandler 
{

    long timeout_sec; //! Timeout, seconds
    long timeout_usec; //! Timeout, microseconds

      //! Are we going to go down once the output queue's empty
    bool scheduleddown; 

protected: 

      //! Pointer to the SUEEventSelector object used here. 
    SUEEventSelector *the_selector;

      //! Reading queue
    SUEBuffer inputbuffer;
      //! Writing queue
    SUEBuffer outputbuffer;


      //! Does the received input reset timeout
    bool inputresetstimeout;
      //! Does the performed output reset timeout
    bool outputresetstimeout;

      //! callback function for file descriptor events
      /*! This function overrides SUEFdHandler::FdHandle().
          It then performs read(2) on the socket and then calls
          the appropriate method depending on the results. 
          \note Forget this function unless you'd like to 
          reimplement the library. For regular modifying of
          your object's behaviour, use overriding of HandleNewInput(),
          HandleSessionTimeout(), HandleRemoteClosing() and 
          HandleReadError().
       */ 
    virtual void FdHandle(bool a_r, bool a_w, bool a_ex);

      //! callback function for timeouts
      /*!
          \note Forget this function unless you'd like to 
          reimplement the library. For regular modifying of
          your object's behaviour, override HandleSessionTimeout().
        */
    virtual void TimeoutHandle();
  
      
      /*! We're always ready to read everything they sent us */
    virtual bool WantRead() const { return true; }
      /*! Sometimes (when the output queue is not empty) we need to 
        know when to write. 
       */
    virtual bool WantWrite() const { return outputbuffer.Length() > 0; }
      /*! In the current version, we don't expect nor handle exceptions */
    virtual bool WantExcept() const { return false; }

public:
      //! Constructor
      /*! 
          \param a_timeout is the value of the timeout (in seconds)
                 used to close idle sessions. 0 means no timeout. 
                 Use SetTimeout() to set timeouts of non-integer seconds.
          \param a_greeting is the message to send to the client 
                 at the very start of the session.
       */
    SUEGenericDuplexSession(int a_timeout = 0,
                            const char *a_greeting = 0);    
      //! Destructor
    virtual ~SUEGenericDuplexSession();

      //! Set timeout
      /*! \param sec is integer seconds
          \param usec is additional microseconds
          \note SetTimeout(0,0) will disable the timing-out feature.
          \note Setting timeout for an active (registered with the
          selector) session forces timeout reset.
       */
    void SetTimeout(int sec, int usec = 0);

      //! Start the session
      /*! This method sets the file descriptor and registers the 
          handlers at the selector. 
          \param a_selector is the pointer to your SUEEventSelector
          \param a_fd is the file descriptor of your duplex connection
          \param a_greeting if specified, this string is sent to the 
                 remote end right after the connection is established
       */
    void Startup(SUEEventSelector *a_selector, 
                 int a_fd, const char *a_greeting = 0);

      //! Shut the session down
      /*! Calling Shutdown() from within your handlers is the right way to
          end the session. This function unregisters all handlers at the 
          selector and closes the file descriptor. After that, it calls 
          ShutdownHook() method which can be overriden in a child class. 
          \note If the session is not started up yet or is already shutted
          down, Shutdown() silently exits. ShutdownHook() is not called in 
          this case. This allows to call Shutdown() from the destructor 
          of SUEGenericDuplexSession to make sure everything's cleaned up.
       */
    void Shutdown();

      //! graceful shutdown
      /*! Schedule shutdown of the session to the moment when 
          output queue get empty. 
       */
    void GracefulShutdown();

      //! Hook for handling new input
      /*! This function is called whenever new data is read from 
          the session channel (that is, select(2) told us there
          is possibility to read, and read(2) got more than zero
          chars from the session's file descriptor). 
          \par
          All the chars are appended to the inputbuffer before
          this function is called.
       */                
    virtual void HandleNewInput() = 0; 

      //! Overridable shutdown hook
      /*! Sometimes we need to notify someone the session's dead.
          This function is called by Shutdown() method after all 
          the necessary steps of shutting down are done. 
          By default it does nothing, but that can be overriden.
          You can even delete your object from within this method,
          it is safe unless you use deletion to initiate the 
          shutdown.
          \note If you assume deletion of the object is a regular
          method of killing your session, be sure to call Shutdown()
          from within your destructor.  
       */ 
    virtual void ShutdownHook() {} 

      //! Hook for handling session timeout
      /*! This function is called when the session is timed out.
          By default, it calls Shutdown(). Override this method if
          you need different behaviour (you do need it, don't you?)    
       */ 
    virtual void HandleSessionTimeout() { Shutdown(); }

      //! Hook for handling read error
      /*! This function is called when read(2) returns negative value.
          By default, it calls Shutdown(). Override this method if
          you need different behaviour (you do need it, don't you?)    
       */ 
    virtual void HandleReadError() { Shutdown(); }

      //! Hook for handling write  error
      /*! This function is called when write(2) returns negative value.
          By default, it calls Shutdown(). Override this method if
          you need different behaviour (you do need it, don't you?)    
       */ 
    virtual void HandleWriteError() { Shutdown(); }

      //! Hook for handling session end by remote
      /*! This function is called by the library in case select(2) 
          tells the FD may be read and then read(2) returns 0 bytes. 
          For sockets, it means the remote end has just closed connection.
       */ 
    virtual void HandleRemoteClosing() { Shutdown(); }

private:
    void ResetTimeout();
};


#endif //sentry
