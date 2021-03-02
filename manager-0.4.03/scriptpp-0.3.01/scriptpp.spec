Name: scriptpp
Version: 0.3.00
Release: 0

Summary: Advanced string processing (script-like) within C++
License: LGPL
Group: System/Libraries
Url: http://www.croco.net/software/scriptpp/

Source: ftp://ftp.croco.net/pub/software/croco/scriptpp/%name-%version.tgz
BuildRequires: gcc-c++

%description
The library provides a C++ class named ScriptVariable which is a good
replacement for well-known "standard" string class.  Besides usual
operations, it provides operations of tokenization, breaking a string down
to words, and conversion from/to integers and floats.  Operations which
need a range, such as copying, erasing or replacing a range, are
implemented in a more object-oriented style than for the string class.
Certainly, the copy-on-write technique is implemented.

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
