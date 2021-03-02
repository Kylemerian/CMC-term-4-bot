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
#include <unistd.h>
#include "sue_sess.hpp"




SUEBuffer::SUEBuffer()
{
    data = new char[maxlen = 512];
    datalen = 0;
}

SUEBuffer::~SUEBuffer()
{
    delete[] data;
}

void SUEBuffer::AddData(const char *buf, int size)
{
    ProvideMaxLen(datalen + size);
    memcpy(data + datalen, buf, size);
    datalen += size;
}

void SUEBuffer::AddChar(char c)
{
    ProvideMaxLen(datalen + 1);
    data[datalen] = c;
    datalen++;
}

int SUEBuffer::GetData(char *buf, int size)
{
    if(datalen == 0)
        return 0; // a bit of optimization ;-)
    if(size >= datalen) { // all data to be read/removed
        memcpy(buf, data, datalen);
        int ret = datalen;
        datalen = 0;
        return ret;
    } else { // only a part of the data to be removed
        memcpy(buf, data, size);
        DropData(size);
        return size;
    }
}

void SUEBuffer::DropData(int size)
{
    EraseData(0, size);
}

void SUEBuffer::EraseData(int index, int size)
{
    if(index+size>=datalen) {
        datalen = index;
    } else {
        memmove(data+index, data+index+size, datalen - (index+size));
        datalen -= size;
    }
}

void SUEBuffer::AddString(const char *str)
{
    AddData(str, strlen(str));
}


int SUEBuffer::ReadLine(char *buf, int bufsize)
{
    int crindex = -1;
    for(int i=0; i< datalen; i++)
        if(data[i] == '\n') { crindex = i; break; }
    if(crindex == -1) return 0;
    if(crindex >= bufsize) { // no room for the whole string
        memcpy(buf, data, bufsize-1);
        buf[bufsize-1] = 0;
        DropData(crindex+1);
    } else {
        GetData(buf, crindex+1);
        //assert(buf[crindex] == '\n');
        buf[crindex] = 0;
        if(buf[crindex - 1] == '\r')
            buf[crindex-1] = 0;
    }
    return crindex + 1;
}

bool SUEBuffer::ReadLine(SUEBuffer &dest)
{
    int crindex = -1;
    for(int i=0; i< datalen; i++)
        if(data[i] == '\n') { crindex = i; break; }
    if(crindex == -1) return false;
    dest.ProvideMaxLen(crindex+1);
    GetData(dest.data, crindex+1);
    dest.datalen = crindex+1;
    //assert(dest.data[crindex] == '\n');
    dest.data[crindex] = 0;
    if(dest.data[crindex-1] == '\r')
        dest.data[crindex-1] = 0;
    return true;
}

int SUEBuffer::FindLineMarker(const char *marker) const
{
    for(int i = 0; i< datalen; i++) {
        if(data[i] == '\r') i++;
        if(data[i] != '\n') continue;
        bool found = true;
        int j = 1;
        for(const char *p = marker;*p;p++,j++) {
            if(/* data[i+j]=='\0' ||*/ data[i+j] != *p)
                { found = false; break; }
        }
        if(!found) continue;
        if(data[i+j] == '\r') j++;
        if(data[i+j] != '\n') continue;
        return i+j;
    }
    return -1;
}

int SUEBuffer::ReadUntilLineMarker(const char *marker, char *buf, int bufsize)
{
    int ind = FindLineMarker(marker);
    if(ind == -1) return -1;
    if(bufsize >= ind+1) {
        return GetData(buf, ind+1);
    } else {
        return GetData(buf, bufsize);
    }
}

bool SUEBuffer::ReadUntilLineMarker(const char *marker, SUEBuffer &dest)
{
    int ind = FindLineMarker(marker);
    if(ind == -1) return false;
    dest.ProvideMaxLen(ind+2);
    GetData(dest.data, ind+1);
    dest.data[ind+1] = 0;
    dest.datalen = ind+1;
    return true;
}

bool SUEBuffer::ContainsExactText(const char *str) const
{
    int i;
    for(i=0; (i<datalen) && (str[i]!=0); i++) {
        if(str[i]!=data[i]) return false;
    }
    return (i==datalen) && (str[i]==0);
}

void SUEBuffer::ProvideMaxLen(int n)
{
    if(n <= maxlen) return;
    int newlen = maxlen;
    while(newlen < n) newlen*=2;
    char *newbuf = new char[newlen];
    memcpy(newbuf, data, datalen);
    delete [] data;
    data = newbuf;
    maxlen = newlen;
}



// SUEGenericDuplexSession

void SUEGenericDuplexSession::TimeoutHandle()
{
    HandleSessionTimeout();
}

void SUEGenericDuplexSession::FdHandle(bool a_r, bool a_w, bool /*a_ex*/)
{
    if(a_r) {
        char buf[1024];
        int rb = read(fd, buf, sizeof(buf));
        if(rb>0) {
            if(inputresetstimeout)
                ResetTimeout();
            inputbuffer.AddData(buf, rb);
            HandleNewInput();
        } else if(rb==0) {
            HandleRemoteClosing();
            return;
        } else {
            HandleReadError();
        }
    }
    if(a_w) {
        int len = outputbuffer.Length();
        if(len) {
            int wb = write(fd, outputbuffer.GetBuffer(), len);
            if(wb>0) { 
                if(outputresetstimeout)
                    ResetTimeout();
                outputbuffer.DropData(wb);
            } else if(wb==-1) {
                HandleWriteError();
            } else {
                 /* simply ignore this situation */
            }
            if(scheduleddown && outputbuffer.Length() == 0) 
                 Shutdown();
        }
    }
}

SUEGenericDuplexSession::SUEGenericDuplexSession( 
                            int a_timeout, 
                            const char *a_greeting) 
   : SUEFdHandler(), SUETimeoutHandler()  
{
    the_selector = 0;  
    timeout_sec = a_timeout;
    timeout_usec = 0;
    if(a_greeting) {
        outputbuffer.AddData(a_greeting, strlen(a_greeting));
    }
    scheduleddown = false;
    inputresetstimeout = true;
    outputresetstimeout = true;
}

SUEGenericDuplexSession::~SUEGenericDuplexSession()
{
    Shutdown();
}

void SUEGenericDuplexSession::Startup(SUEEventSelector *a_selector, 
                                      int a_fd, const char *a_greeting)
{
    the_selector = a_selector;
    if(a_greeting) {
        outputbuffer.DropAll();
        outputbuffer.AddData(a_greeting, strlen(a_greeting));
    }
    SetFd(a_fd);
    the_selector->RegisterFdHandler(this);
    if(timeout_sec>0 || timeout_usec>0) {
        SetFromNow(timeout_sec, timeout_usec);
        the_selector->RegisterTimeoutHandler(this);
    }
}

void SUEGenericDuplexSession::Shutdown()
{
    if(the_selector) {
        the_selector->RemoveFdHandler(this);
        the_selector->RemoveTimeoutHandler(this);
    }
    the_selector = 0;
    if(fd != -1) {
        close(fd);
        SetFd(-1);
        ShutdownHook();
    }
}

void SUEGenericDuplexSession::GracefulShutdown()
{
    scheduleddown = true;
}

void SUEGenericDuplexSession::ResetTimeout()
{
    the_selector->RemoveTimeoutHandler(this);
    if(timeout_sec>0 || timeout_usec>0) {
        SetFromNow(timeout_sec, timeout_usec);
        the_selector->RegisterTimeoutHandler(this);
    }
}

void SUEGenericDuplexSession::SetTimeout(int sec, int usec)
{
    timeout_sec = sec;
    timeout_usec = usec;
    if(the_selector) ResetTimeout();
}
