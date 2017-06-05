#
#    zm-proto - Basic messaging for zmon.it
#
#    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
#    of zmon.it, the fast and scalable monitoring system.                           
#                                                                                   
#    This Source Code Form is subject to the terms of the Mozilla Public License, v.
#    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
#    one at http://mozilla.org/MPL/2.0/.                                            
#

# To build with draft APIs, use "--with drafts" in rpmbuild for local builds or add
#   Macros:
#   %_with_drafts 1
# at the BOTTOM of the OBS prjconf
%bcond_with drafts
%if %{with drafts}
%define DRAFTS yes
%else
%define DRAFTS no
%endif
Name:           zm-proto
Version:        1.0.0
Release:        1
Summary:        basic messaging for zmon.it
License:        MIT
URL:            http://example.com/
Source0:        %{name}-%{version}.tar.gz
Group:          System/Libraries
# Note: ghostscript is required by graphviz which is required by
#       asciidoc. On Fedora 24 the ghostscript dependencies cannot
#       be resolved automatically. Thus add working dependency here!
BuildRequires:  ghostscript
BuildRequires:  asciidoc
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  xmlto
BuildRequires:  zeromq-devel
BuildRequires:  czmq-devel
BuildRequires:  malamute-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
zm-proto basic messaging for zmon.it.

%package -n libzm_proto1
Group:          System/Libraries
Summary:        basic messaging for zmon.it shared library

%description -n libzm_proto1
This package contains shared library for zm-proto: basic messaging for zmon.it

%post -n libzm_proto1 -p /sbin/ldconfig
%postun -n libzm_proto1 -p /sbin/ldconfig

%files -n libzm_proto1
%defattr(-,root,root)
%{_libdir}/libzm_proto.so.*

%package devel
Summary:        basic messaging for zmon.it
Group:          System/Libraries
Requires:       libzm_proto1 = %{version}
Requires:       zeromq-devel
Requires:       czmq-devel
Requires:       malamute-devel

%description devel
basic messaging for zmon.it development tools
This package contains development files for zm-proto: basic messaging for zmon.it

%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/libzm_proto.so
%{_libdir}/pkgconfig/libzm_proto.pc
%{_mandir}/man3/*
%{_mandir}/man7/*

%prep
%setup -q

%build
sh autogen.sh
%{configure} --enable-drafts=%{DRAFTS}
make %{_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

%files
%defattr(-,root,root)
%{_bindir}/zmpub
%{_mandir}/man1/zmpub*
%{_bindir}/zmsub
%{_mandir}/man1/zmsub*

%changelog
