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




#ifndef SENTRY_SUE_SEL_HPP
#define SENTRY_SUE_SEL_HPP

/*! \file sue_sel.hpp
    \brief The SUE library base
   
    This file implements the main classes of the library: 
     - SUEEventSelector
     - SUEFdHandler
     - SUETimeoutHandler
     - SUESignalHandler
     - SUELoopHook
     - SUEException
    These classes are likely needed in almost any event-driven application
 */

//! Event selector
/*! This class provides an object-oriented framework for the unix select(2)
    system call. It uses objects of:
     - SUEFdHandler class for file descriptor oriented events, 
     - SUETimeoutHandler class for time events,
     - SUESignalHandler class for signal events,
     - SUELoopHook to handle various postponed tasks (called
            on every iteration of the main loop)
    You need to create an object of this class, then decide what events you 
    wish to handle from the very start, create the appropriate objects 
    for these events, register them using the appropriate methods, then
    call Go() and enjoy. Certainly you can register more events and 
    unregister some of the registered events at any time you want, but 
    please note if you define no events before calling Go() then it will
    loop forever. Remember, you're creating an event-driven application!
    \note This class is THE good point to start with. You *ALWAYS* need 
      one (and in almost all cases only one) object of this class. 
      The Go() method of this class is the main loop of your application,
      and the Break() method is used to break the main loop.
    \note You can use this class as it is to create your Selector object.
      You might want to create a subclass when you need to use 
      sets of handlers somewhere outside of the class (e.g., for debugging
      purposes), or if you want to handle select(2) failures specifically
      via overriding the HandleSelectFailure() method. 
 */
class SUEEventSelector {
         //! Timeouts queue (sorted by time, earlier first)
    struct TimeoutsListItem {
        TimeoutsListItem *next;
        class SUETimeoutHandler *handler; 
    } *timeouts;
         //! Signal wanters list
    struct SignalListItem {
        SignalListItem *next;
        class SUESignalHandler *handler;
    } *signalhandlers;
         //! File descriptor handlers 
         /*! Array indexed by descriptor values themselves.
             That is, fdhandlers[3] is a pointer to the FdHandler
             for fd=3, if any, or NULL. Array is resized as necessary.
          */
    class SUEFdHandler** fdhandlers;
         //! Current size of the fdhandlers array
    int fdhandlerssize;
         //! List of loop hooks
    struct LoopHookListItem {
        LoopHookListItem *next;
        class SUELoopHook *hook; 
    } *loophooks;
         //! Is it time to break the main loop?
         /*! This flag is cleared by Go() function and may be set by
             Break() function. Go() checks the flag after all the 
             handlers are called, and if it is set, the main loop is
             broken (Go() returns instead of doing next iteration).
          */
    bool breakflag;
private:
         //! Resize (enlarge) the fdhandlers array
    void ResizeFdHandlers(int newsize);
public:
       //! Constructor
    SUEEventSelector();
       //! Destructor
    virtual ~SUEEventSelector();
       //! Register file descriptor handler
       /*! Registers an FD to be watched. FD is specified with a 
           SUEFdHandler subclass object. SUEFdHandler provides
           the value of the descriptor, the notification modes (r/w/e), 
           and the callback function for notification (as a virtual method).
           \note The FD handler remains registered unless it is explicitly 
           unregistered with RemoveFdHandler method.
           \warning It is assumed FD doesn't change when the handler is 
           registered.
        */
    void RegisterFdHandler(SUEFdHandler *h);
       //! Removes the specified handler
       /*! Removes (unregisters) an FD handler. In case the handler is
           not registered, silently ignores the call
        */
    void RemoveFdHandler(SUEFdHandler *h);
       
       //! Register a timeout handler
       /*! Registers a time moment at which to wake up and call the 
           notification method. Time value (in seconds and microseconds)
           is specified by a SUEFdHandler subclass object which also 
           provides a callback function for notification (as a 
           virtual method). 
           \note The timeout handler remains registered unless 
           either the moment comes and notification function is called
           OR it is explicitly unregistered with RemoveTimeoutHandler
           method.
           \warning It is assumed that the value of the timeout doesn't
           change when it is registered. Changind it will lead to 
           unpredictable behaviour. If you need to change thee timeout, 
           then first unregister it, then change and register again.
        */
    void RegisterTimeoutHandler(SUETimeoutHandler *h);
       //! Removes the specified handler
       /*! Removes (unregisters) a timeout handler. In case the handler is
           not registered, silently ignores the call
        */
    void RemoveTimeoutHandler(SUETimeoutHandler *h);


       //! Register a signal handler
       /*! Registers a signal specified with SUESignalHandler object  
        */
    void RegisterSignalHandler(SUESignalHandler *h);
       //! Remove a signal handler
    void RemoveSignalHandler(SUESignalHandler *h);
	
       //! Register a loop hook
    void RegisterLoopHook(SUELoopHook *h);
       //! Remove a signal handler
    void RemoveLoopHook(SUELoopHook *h);
	
       //! Main loop
       /*! This function 
            - sets up the fd_set's for read, write and except notifications
              in accordance to the set of registered file handlers
            - chooses the closest time event from the set of registered
              timeout handlers
            - sets the signal handling functions as appropriate
            - calls select(2)
            - checks if any handled signals happened
            - examines the fd_set's for FDs changed state and calls the
              appropriate callback methods
            - examines the timeouts list, selects those are now in 
              the past, removes each of them from the list of registered 
              objects and calls its notification method
            - checks for the loop breaking flag. If it is set, breaks the 
              loop and returns. Otherwise, continues from the beginning.

           \par
           The function is intended to be the main loop of your process.   
        */
    void Go();   
       //! Cause main loop to break (Go() function to return)
       /*! This function sets the loop breaking flag which causes
           Go() function to return. The flag is cleared each time 
           Go() is called. Go() checks for this flag after the main sequence
           of actions is performed, before repeating it. 
        */
    void Break();
	
       //! Handle select(2) errors
       /*! This method is called whenever select(2) returns negative
           value.
           \param rc is the value returned by select(2)
           \note override this method to handle the event yourself
        */
    virtual void HandleSelectFailure(int rc) {}
private:
    // several private functions used to decomposite Go()
    struct timeval* ComputeClosestTimeout(struct timeval &timeout);
    void SetupFdSets(struct SelectDescriptorsSet &);
    void HandleSignals();
    void HandleFds(struct SelectDescriptorsSet &);
    void HandleTimeouts(struct timeval &current);
    void HandleLoopHooks();
};

//! File descriptor handler for SUEEventSelector
/*!
  When we've got a file descriptor to be handled with select() call
  (that is, with SUEEventSelector class), we need to create a subclass of
  SUEFdHandler overriding void FdHandle(bool, bool, bool) virtual method.
  Then, create an object of that class and register it with the 
  SUEEventSelector object. 
  \warning It is assumed that the descriptor doesn't change after the object
  is registered. If you need to change it, then unregister the object, 
  change the descriptor and then register the object again. 
*/
class SUEFdHandler {
    friend class SUEEventSelector;
protected:
    int fd;               //!< File descriptor to handle
public:
      //! Constructor
      /*! Constructor of the class. 
          \param a_fd is the file descriptor
          \note File descriptor should not be changed when the handler is 
            installed. 
       */
    SUEFdHandler(int a_fd = -1); 
      //! Destructor
      /*! Destructor of the class
          \warning It is the user's duty to make sure the object is
               NO LONGER REGISTERED with the selector before 
               destroying the object
       */
    virtual ~SUEFdHandler();

      //! Callback function for notification.
      /*! The SUEEventSelector calls this method when the file descriptor
          changes it's status to notify us about it.
          \note This method MUST be overriden in a subclass 
          in order to provide the appropriate functionality.
       */
    virtual void FdHandle(bool a_r, bool a_w, bool a_ex) = 0; 

       //! Set/change the file descriptor value
       /*! This method is intended to be used in case we don't know
           the actual descriptor's value at the moment of creation 
           of the object so that we can't pass it to the constructor. 
           \note This must be done BEFORE we register the object with the
           UPEventSelector object.
           \warning Calling this method after the handler is registered 
           with the selector will lead to unpredictable effects. 
       */
    void SetFd(int a_fd) { fd = a_fd; }

      //@{
      //! Do we want to read?
      /*! Should the selector call this object's handler when 
          new data is available for reading?
       */
    virtual bool WantRead() const { return true; }
      //! Do we want to write?
      /*! Should the selector call this object's handler when 
          it is safe to write (that is, write(2) will not block)?
       */
    virtual bool WantWrite() const { return false; }
      //! Do we want exceptions?
      /*! Should the selector call this object's handler when 
          an exception happens to the descriptor?
       */
    virtual bool WantExcept() const { return false; }
      //@}
};



//! Timeout handler for SUEEventSelector
/*! When we wish the SUEEventSelector to notify us that a given time event
    happens, we create a sublcass of SUETimeoutHandler and register it with
    the SUEEventSelector object. 
    \note The SUETimeoutHandler object remains registered unless the event
    happens and the notification is done OR it is unregistered explicitly. 
    \warning Changing the timeout value when the object is registered will
    lead to unpredictable behaviour. 
 */
class SUETimeoutHandler {
    friend class SUEEventSelector;
       //! seconds since epoch when the timeout is to happen
    long sec;  
       //! microseconds (in addition to sec)
    long usec;
public:
      //! Constructor of a full form
      /*! This form of constructor allows to pass the actual moment 
          to notify us at.
       */
    SUETimeoutHandler(long a_sec, long a_usec);

      //! Default constructor
      /*! This form assumes we'll set the timeout value later with Set
          or SetFromNow methods
       */
    SUETimeoutHandler();

      //! Destructor
      /*! \warning it's the user's duty to make sure the object is no longer
          registered before destroying it
       */
    virtual ~SUETimeoutHandler();

      //! Set the timeout in absolute time
      /*! Set the actual moment when we need to be notified. 
          \param a_sec seconds since epoch when the timeout is to happen
          \param a_usec microseconds (in addition to sec)
       */
    void Set(long a_sec, long a_usec);

      //! Set the timeout relative to the current time
      /*! Set the time interval after which the timeout is to happen.
          The method actually calls gettimeofday(3) and adds the arguments
          to the values returned.
          \param a_sec - seconds
          \param a_usec - microseconds
       */
    void SetFromNow(long a_sec, long a_usec = 0);

      //! Read the value. Used primarily by SUEEventSelector
    void Get(long &a_sec, long &a_usec) const;

      //! Read the value, call gettimeofday(3) and return the difference
    void GetRemainingTime(long &a_sec, long &a_usec) const;

      //! Compare two timevalues. Used primarily by SUEEventSelector
    bool operator < (const SUETimeoutHandler &other) const
     { return (sec < other.sec)||(sec == other.sec && usec < other.usec); }

      //! Is it before the given moment          
    bool IsBefore(long a_sec, long a_usec) const
     { return (sec < a_sec)||(sec == a_sec && usec < a_usec); }
 
      //! Callback function for notification.
      /*! The SUEEventSelector calls this method when the 
          specified moment comes. After that, we can assume the 
          object is no longer registered so it is possible to 
          set a new value and register it again, even from within the
          TimeoutHandle() function itself (because SUEEventSelector 
          unregisters the handler before calling the method). 
          \note This method MUST be overriden in a subclass 
          in order to provide the appropriate functionality.
       */
    virtual void TimeoutHandle() = 0; 
};




//! Signal handler for SUEEventSelector
/*! When we wish the SUEEventSelector to catch some signals and notify us
    we create a SUESignalHandler object. There can be several objects
    for the same signal. If this is the case, then callback functions will
    be called for all the objects once the signal came. 
 */
class SUESignalHandler {
    friend class SUEEventSelector;
protected:
    int signo;
public:
    SUESignalHandler(int a_sig) : signo(a_sig) {}
    virtual ~SUESignalHandler() {}

    virtual void SignalHandle() = 0;
};



//! Loop hook for SUEEventSelector
/*! This class is to be used in case we have (or can have) something
    to be done at the end of each main loop iteration, such as to perform
    tasks postponed from various event handlers.
 */
class SUELoopHook {
    friend class SUEEventSelector;
public:
    SUELoopHook() {}
    virtual ~SUELoopHook() {}

        //! This method is called at the end of each main loop iteration
    virtual void LoopHook() = 0;
};




//! Exception
/*! In case of disasters, the SUE library throws 
    an exception of this type.  
*/
class SUEException {
    const char *s;
public:
    SUEException(const char* a) : s(a) {}
    SUEException() : s("") {}
    SUEException(const SUEException &e) : s(e.s) {}
    ~SUEException() {}
    const char* Get() { return s; }
};



#endif // sentry
