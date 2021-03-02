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
CXXFLAGS = -g -Wall -O2 -I.. 

PROGS = sleep5 sitter chat hellobot sigs children

all:	$(PROGS)

../libsue.a:	FORCE	
	cd .. && $(MAKE) libsue.a

%:	%.cpp ../libsue.a
	$(CXX) $(CXXFLAGS) $< ../libsue.a -o $@

clean:
	rm -f *.o *~ $(PROGS)

deps.mk: $(wildcard *.cpp) Makefile
	$(CXX) -MM $(wildcard *.cpp) -I.. > $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

FORCE:
