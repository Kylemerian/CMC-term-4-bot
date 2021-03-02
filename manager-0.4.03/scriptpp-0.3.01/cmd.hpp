// +-------------------------------------------------------------------------+
// |                     Script Plus Plus vers. 0.3.01                       |
// | Copyright (c) Andrey Vikt. Stolyarov <crocodil_AT_croco.net>  2003-2011 |
// | ----------------------------------------------------------------------- |
// | This is free software.  Permission is granted to everyone to use, copy  |
// |        or modify this software under the terms and conditions of        |
// |                 GNU LESSER GENERAL PUBLIC LICENSE, v. 2.1               |
// |     as published by Free Software Foundation (see the file LGPL.txt)    |
// |                                                                         |
// | Please visit http://www.croco.net/software/scriptpp to get a fresh copy |
// | ----------------------------------------------------------------------- |
// |   This code is provided strictly and exclusively on the "AS IS" basis.  |
// | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
// +-------------------------------------------------------------------------+




#ifndef SCRIPTPP_CMD_HPP_SENTRY
#define SCRIPTPP_CMD_HPP_SENTRY

class ScriptVariable;
class ScriptVector;

class ReadStream {
    void *f;
public:
    ReadStream() : f(0) {}

    bool FOpen(const char *fname);
    bool FDOpen(int fd);
    void FClose();

    bool IsOpen() const { return f != 0; }

    bool ReadLine(ScriptVariable &buf);
    bool ReadLine(ScriptVector &buf, int words = 0,
                 const char *delimiters = " \n\r\t",
                 const char *trimspaces = 0);
};

class ReadText : public ReadStream {
public:
    ReadText(const char *filename) {
        FOpen(filename);
    }
    ~ReadText() { if(IsOpen()) FClose(); }
};

class ExecProgram {
protected:
    int pid;
    int status;
    int save_pgrp;
public:
    ExecProgram();
    ~ExecProgram();

    bool CheckChild();
    void WaitChild();
    void KillChild(int signo = 15);

    bool Success() const;

    class ForkFailed {};
    class PipeFailed {};
};



class ExecResultParse : public ReadStream, public ExecProgram {
public:
    ExecResultParse(const char *path, ...);
    ~ExecResultParse();
};

class ExecAndWait : public ExecProgram {
public:
    ExecAndWait(const char *path, ...);
    ExecAndWait(char * const * argv);
    ~ExecAndWait();
};

class ChrootExecWait : public ExecProgram {
public:
    ChrootExecWait(const char *root, const char *path, ...);
    ~ChrootExecWait();
};


class FileStat {
    void *stat_info;
public:
    FileStat(const char *filename, bool dereference = true);
    ~FileStat();

    bool Exists() const;
    bool IsDir() const;
    bool IsRegularFile() const;
    bool IsSymlink() const;
    bool IsChardev() const;
    bool IsBlockdev() const;
    void GetMajorMinor(int &majn, int &minn) const;
    bool IsEmpty() const;
};


class ReadDir {
    void *dir;
public:
    ReadDir(const char *path);
    ~ReadDir();
    const char* Next();
};


class PreserveTerminalMode {
    void *p;
public:
    PreserveTerminalMode();
    ~PreserveTerminalMode();
};


#endif
