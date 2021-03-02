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




#include <sys/types.h> // for pid_t

#include "sue_sel.hpp"


class SUEChildWaitAgent;

//! SUE Child (SIGNCHLD on particular process) Handler
/*! This class allows to handle the event 'particular child process finished'.
    You need to create a derived class with the ChildHandle() method
    overriden to implement your custom handling. You also can use
    SUEChildHandle as is if you only need to get rid of zombies and no
    special hanling is required.
    \note You need to have an object of SUEChildWaitAgent class in 
    your program in order this mechanism to work. 
    \note Objects of SUEChildHandler class are 'single-use only'.
    You can just create your object with operator new, and all 
    the care is then taken by SUEChildWaitAgent. The object is 
    deleted immediately after the ChildHandle() method is called.
    \warning Never try to create an object of SUEChildHandler 
    anyhow but with operator new! It will not work.
    \warning If you pass an invalid pid, the mechanism will throw
    an exception once your main process get a SIGCHLD.
 */
class SUEChildHandler {
    int status;
    pid_t pid;
public:
        //! Constructor
    SUEChildHandler(pid_t a_pid, SUEChildWaitAgent *a_agent);
        //! Destructor
    virtual ~SUEChildHandler() {}

        //! Handling method
        /*! Override this in your derived class to implement
            your custom functionality. Use GetPid(), IfExited(),
            ExitCode() and IfSignaled() methods to analyse the 
            situation.
            \note The default is to do nothing, which also makes 
            sence because the agent does waitpid(2) on the 
            process thus removing zombie process as approproate.
         */
    virtual void ChildHandle() {}
    
protected:
    pid_t GetPid() const { return pid; }
        //! Did the process exit?
	/*! Only useful from within ChildHandle() */
    bool IfExited() const;
        //! What was the process' exit code?
	/*! Only useful from within ChildHandle(),
	    and only if IfExited() returns true.
	 */
    int ExitCode() const;
        //! What was the process killed with a signal?
	/*! Only useful from within ChildHandle() */
    bool IfSignaled() const;
        //! What was the process' termination signal?
	/*! Only useful from within ChildHandle(),
	    and only if IfSignaled() returns true.
	 */
    int TermSig() const;
    
private:
    friend class SUEChildWaitAgent;
    void AgentsHandler(int a_status) {
        status = a_status;
        ChildHandle();
        pid = 0;
        delete this;
    }
};

//! Child Wait Agent
/*! This object allows the mechanism of SUE child handling support 
    to work. It installs a handler for the SIGCHLD signal, and then 
    searches for the approproate handler of SUEChildHandler type, 
    if any, to call their respective methods.
    \note Perhaps you need only one object of this class in your
    application, but you can create more, register them all with 
    your selector and they will work for you, altough it just slows
    down your application a bit.
    \warning Register this object with your selector BEFORE launching 
    any children! or else the mechanism could work inaccurate. It is 
    also better not to launch any handled children before the main loop
    is started, since it raises the risk of a race condition
 */
class SUEChildWaitAgent : public SUESignalHandler {
    struct ChildListItem {
        ChildListItem *next;
        SUEChildHandler *handler;
    } *first;
    SUEEventSelector *the_selector;
public:
       //! Constructor
    SUEChildWaitAgent();
       //! Destructor
    virtual ~SUEChildWaitAgent();
    
       //! Register it with the selector
    void Register(SUEEventSelector *a_selector);
       //! Remove from the selector
    void Unregister();

       //! Add a child handler
       /*! \note The SUEChildHandler does it itself, 
	   so there's nothing to worry about; normally,
	   you never need this method.
	*/
    void RegisterChildHandler(SUEChildHandler *h);
       //! Remove a previously added child handler
       /*! \note Exited children's handlers are removed
	   (and deleted) automatically, so this method is
	   rarely needed: only in case you fro some reason 
	   don't want to handle a particular child anymore.
	   For example, if you want to change the handling 
	   object.
	*/  
    void RemoveChildHandler(SUEChildHandler *h);
private:
    virtual void SignalHandle();
};


