# Generic library install file
#
# This file assumes the following variables are defined: LIBNAME       --
# the name of the library HEADERFILES   -- header files to be installed
# LIBFILES      -- library files to be instaled
#
# There must be a file named Version, in which the first word of the first
# line must be the version number, such as 0.5.17
#
# Two modes of the install are supported: INSTALLMODE=native        install
# right into $(PREFIX)/{lib,include} INSTALLMODE=separate      (default)
# create a separate directory
#
# In order to prepare a package, DESTDIR can be specified.  In this case,
# all other directories are relative to DESTDIR. Mode must be native in
# order this to work.


VERSION_SUFFIX=$(word 1, $(shell head -1 Version))

######
# Various install tree descriptions

ifeq ($(INSTALLMODE),native) 

    # 'native' install (choose ONE prefix)
PREFIX=/usr/local
#PREFIX=/usr
SYMLINK_PREFIX=
INCLUDEDIR=$(PREFIX)/include/$(LIBNAME)
LIBDIR=$(PREFIX)/lib
DOCDIR=$(PREFIX)/share/docs

else

# 'separate directory' install
SYSPREFIX=/usr/local
PREFIX=$(SYSPREFIX)/$(LIBNAME)-$(VERSION_SUFFIX)
ifneq ($(findstring -$(VERSION_SUFFIX),$(PREFIX)),)
  SYMLINK_PREFIX=$(subst -$(VERSION_SUFFIX),,$(PREFIX))
else
  SYMLINK_PREFIX=
endif
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
DOCDIR=$(PREFIX)/share/docs

endif


INSTALL = install
INSTALL_DIR = $(INSTALL) -d
INSTALL_HEADERS = $(INSTALL) --mode=0644
INSTALL_LIB = $(INSTALL) --mode=0644

LDCONFIG = /sbin/ldconfig


install:	$(HEADERFILES) $(LIBFILES)
	$(INSTALL_DIR) $(DESTDIR)$(INCLUDEDIR) 
	$(INSTALL_HEADERS) $(HEADERFILES)  $(DESTDIR)$(INCLUDEDIR) 
	$(INSTALL_DIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL_LIB) $(LIBFILES) $(DESTDIR)$(LIBDIR)
ifndef DESTDIR
ifneq ($(SYMLINK_PREFIX),)
	ln -sfn $(PREFIX) $(SYMLINK_PREFIX)
	for L in $(LIBFILES) ; do\
		ln -sf $(SYMLINK_PREFIX)/lib/$$L $(SYSPREFIX)/lib ;\
	done
	[ -e $(SYSPREFIX)/include/$(LIBNAME) ] || \
		ln -sf $(SYMLINK_PREFIX)/include $(SYSPREFIX)/include/$(LIBNAME)
endif
ifneq ($(LDCONFIG),)
	$(LDCONFIG) $(LIBDIR)
endif
endif
