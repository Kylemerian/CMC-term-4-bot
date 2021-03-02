# +-------------------------------------------------------------------------+
# |                   Manager game server, vers. 0.4.01                     |
# |    Copyright (c) Andrey Vikt. Stolyarov <avst_AT_cs.msu.ru> 2004-2006   |
# | ----------------------------------------------------------------------- |
# | This is free software.  Permission is granted to everyone to use, copy  |
# |        or modify this software under the terms and conditions of        |
# |                     GNU GENERAL PUBLIC LICENSE, v.2                     |
# | as published by Free Software Foundation      (see the file COPYING)    |
# | ----------------------------------------------------------------------- |
# |   This code is provided strictly and exclusively on the "AS IS" basis.  |
# | !!! THERE IS NO WARRANTY OF ANY KIND, NEITHER EXPRESSED NOR IMPLIED !!! |
# +-------------------------------------------------------------------------+




CXXFLAGS = -Wall -g -O0 -I..

LOCALLIBS = -lsue -lscriptpp -Lsue -Lscriptpp 
LIBDEPEND = sue/libsue.a scriptpp/libscriptpp.a

SRCMODULES = manager.cpp gamecoll.cpp mgame.cpp stock.cpp
OBJECTS = $(SRCMODULES:.cpp=.o)

manag:	$(OBJECTS) $(LIBDEPEND)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LOCALLIBS)

sue/libsue.a:
	cd sue && $(MAKE)

scriptpp/libscriptpp.a:
	cd scriptpp && $(MAKE)

%.o:	%.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tags:
	ctags *.[ch]pp


clean:
	rm -f core manag *.o tags
	cd sue && $(MAKE) clean
	cd scriptpp && $(MAKE) clean
