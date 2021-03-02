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




/*
  This program is a TCP client demo. It implements a 'bot' for 
  the chatroom implemented by chat.cpp. 

  It connects to the server running at port 6666 on address 
  127.0.0.1, registers as 'HelloBot' and then, when someone 
  enters the room, the bot greets the newcomer with a greeting 
  message. The event is also shown at the console.

  Besides that, some effort is made to demonstrate the timeout
  handling. When the room remains silent for 20 seconds, the bot
  'yawns' (but it never disconnects until the server closes the
  connection itself).

  The last feature of the bot is that it tries to reconnect when
  the remote end closes the connection. This is useful because the 
  angry server times out anyone who remains silent for 60 seconds.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sue_sel.hpp"
#include "sue_tcpc.hpp"

const char *the_server_address = "127.0.0.1";
const int the_server_port = 6666;
const int the_yawn_timeout = 20;


class HelloBotSession : public SUETcpClientSession {
    bool already_registered;
    bool reconnecting;
public:
    HelloBotSession(const char *ipaddr, int ipport,
                    int timeout)
        : SUETcpClientSession(ipaddr, ipport, timeout)
    { already_registered = false; reconnecting = false; }

    ~HelloBotSession() {}

      // this function must be overriden in every application, 
      // it is the place to implement the main functionality of the session
    virtual void HandleNewInput() {
        SUEBuffer ln;
        if(!already_registered && 
           inputbuffer.ContainsExactText("Please enter your name: "))
        {
             outputbuffer.AddString("HelloBot\n");
             printf("* Registered on the server\n");
             already_registered = true;
        }
        while(inputbuffer.ReadLine(ln)) {
            if(ln[0]=='*') {
                char *name = new char[ln.Length()];
                char *action = new char[ln.Length()];
                sscanf(ln.GetBuffer()+1, "%s %s", name, action);
                if(strcmp(action, "entered")==0) {
                     outputbuffer.AddString("Hello, ");
                     outputbuffer.AddString(name);
                     outputbuffer.AddString("!\n");
                     printf("* Greeting sent to %s\n", name);
                }
                delete[] action;
                delete[] name;
            } 
        }
    }

      // we want to 'yawn' when the silence remains too long, 
      // so we override this function. If we had left it untouched, 
      // it would simply shut the session down.
    virtual void HandleSessionTimeout() {
        outputbuffer.AddString("*YAWN* too long pause, folks\n");
        printf("* YAWN\n");
    }

      // if the session couldn't start, then we'll bever enter the 
      // select loop (see the main() function). But if the session
      // has started somehow, we'll find ourselves in that loop.
      // It is guaranteed, however, that if the session is started, 
      // then this hook will be called exactly one time. So, let's 
      // use it to break the loop, but only in case we're not going
      // to reconnect (note that reconnecting is just a flag of this
      // demo class, it has nothing to do with SUE)
    virtual void ShutdownHook() { 
        if(!reconnecting)
            the_selector->Break();
    }

      // let's try to reconnect instead of giving up
    virtual void HandleRemoteClosing() { 
        reconnecting = true; 
               // prevent ShutdownHook() from breaking the main loop
        Shutdown(); 
        reconnecting = false;

        already_registered = false;
        if(!Up(the_selector)) {
            // well if it fails right here, than don't try anymore
            the_selector->Break();
        }
    }


};




int main(int argc, char **argv)
{
    SUEEventSelector selector;
    HelloBotSession bot(the_server_address, the_server_port, 
                        the_yawn_timeout);
    if(!bot.Up(&selector)) {
        printf("Couldn't start the session\n");
        exit(1);
    }
    selector.Go();
    return 0;
}
