# +-------------------------------------------------------------------------+
# |              (S)imple (U)nix (E)vents vers. 0.2.71                      |
# | Copyright (c) Andrey Vikt. Stolyarov <crocodil_AT_croco.net> 2003-2008. |
# | ----------------------------------------------------------------------- |
# | This is free software.  Permission is granted to everyone to use, copy  |
# |        or modify this software under the terms and conditions of        |
# |                 GNU LESSER GENERAL PUBLIC LICENSE, v. 2.1               |
# | as published by Free Software Foundation      (see the file LGPL.txt)   |
# |                                                                         |
# | Please visit http://www.croco.net/software/sue for a fresh copy of Sue. |
# | ----------------------------------------------------------------------- |
# |   This code is provided strictly and exclusively on the "AS IS" basis.  |
# | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
# +-------------------------------------------------------------------------+




CXX = g++
CXXFLAGS = -g -Wall -O2
CTAGS = /usr/bin/ctags

SOURCES = sue_sel.cpp \
	sue_sess.cpp sue_inet.cpp sue_tcps.cpp sue_tcpc.cpp\
	sue_wait.cpp
HEADERS = $(SOURCES:.cpp=.hpp)
OBJECTS = $(SOURCES:.cpp=.o)


# for install.mk
LIBNAME = sue
LIBFILES = lib$(LIBNAME).a
HEADERFILES = $(HEADERS)	


default: libsue.a
	:


%.o:	%.cpp 
	$(CXX) $(CXXFLAGS) -c $<

libsue.a:	$(OBJECTS)
	$(AR) crs $@ $^ 

tags:	$(SOURCES) $(HEADERS)
	$(CTAGS) $(SOURCES) $(HEADERS) 

doxydocs:
	doxygen suedoxy.conf

clean:
	rm -f *.o *.bin *.a *~ buf deps.mk tags
	rm -rf __doxydocs
	cd demo && $(MAKE) clean

deps.mk: $(SOURCES) Makefile
	$(CXX) -MM $(SOURCES) > $@

ifneq (clean, $(MAKECMDGOALS))
ifneq (tags, $(MAKECMDGOALS))
-include deps.mk
endif
endif

include install.mk
