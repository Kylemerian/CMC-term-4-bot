Name: sue
Version: 0.2.71
Release: 0

Summary: Simple Unix Events: C++ library for event-driven programming
License: LGPL
Group: System/Libraries
Url: http://www.croco.net/software/scriptpp/

Source: ftp://ftp.croco.net/pub/software/croco/scriptpp/%name-%version.tgz
BuildRequires: gcc-c++

%description
Simple Unix Events (SUE) Library is a small and light-weight set of C++
classes which lets you easily create an event-driven application (such as a
single-process TCP server) with C++ under Unix.

#  %package devel
#  Summary: Libraries, includes to develop applications with %name
#  Group: Development/Libraries
#  Requires: %name = %version
#  
#  %description devel
#  The %name-devel package contains the header files and static libraries for
#  building applications which use %name.

%prep
%setup -q

%build
make

%install
make DESTDIR=%buildroot PREFIX=%prefix INSTALLMODE=native install

%post 
%post_ldconfig

%postun
%postun_ldconfig

# %files devel
%files
%_libdir/lib*.a
%dir %_includedir/%name
%_includedir/%name/*.hpp

%changelog
* Tue Feb 06 2007 Andrey Vikt Stolyarov <avst at cs.msu.su> 0.2.50
- the initial version of the spec
