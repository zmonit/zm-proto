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

# build with python_cffi support enabled
%bcond_with python_cffi
%if %{with python_cffi}
%define py2_ver %(python2 -c "import sys; print ('%d.%d' % (sys.version_info.major, sys.version_info.minor))")
%define py3_ver %(python3 -c "import sys; print ('%d.%d' % (sys.version_info.major, sys.version_info.minor))")
%endif

Name:           zm-proto
Version:        0.1.0
Release:        1
Summary:        basic messaging for zmon.it
License:        MPL-2.0
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
%if %{with python_cffi}
BuildRequires:  python-cffi
BuildRequires:  python-devel
BuildRequires:  python-setuptools
BuildRequires:  python3-devel
BuildRequires:  python3-cffi
BuildRequires:  python3-setuptools
%endif
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
zm-proto basic messaging for zmon.it.

%package -n libzm_proto0
Group:          System/Libraries
Summary:        basic messaging for zmon.it shared library

%description -n libzm_proto0
This package contains shared library for zm-proto: basic messaging for zmon.it

%post -n libzm_proto0 -p /sbin/ldconfig
%postun -n libzm_proto0 -p /sbin/ldconfig

%files -n libzm_proto0
%defattr(-,root,root)
%{_libdir}/libzm_proto.so.*

%package devel
Summary:        basic messaging for zmon.it
Group:          System/Libraries
Requires:       libzm_proto0 = %{version}
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
# Install api files into /usr/local/share/zproject
%dir %{_datadir}/zproject/
%dir %{_datadir}/zproject/zm-proto
%{_datadir}/zproject/zm-proto/*.api

%if %{with python_cffi}
%package -n python2-zm-proto_cffi
Group:  Python
Summary:    Python CFFI bindings for zm-proto
Requires:  python = %{py2_ver}

%description -n python2-zm-proto_cffi
This package contains Python CFFI bindings for zm-proto

%files -n python2-zm-proto_cffi
%{_libdir}/python%{py2_ver}/site-packages/zm-proto_cffi/
%{_libdir}/python%{py2_ver}/site-packages/zm-proto_cffi-*-py%{py2_ver}.egg-info/

%package -n python3-zm-proto_cffi
Group:  Python
Summary:    Python 3 CFFI bindings for zm-proto
Requires:  python3 = %{py2_ver}

%description -n python3-zm-proto_cffi
This package contains Python 3 CFFI bindings for zm-proto

%files -n python3-zm-proto_cffi
%{_libdir}/python%{py3_ver}/site-packages/zm-proto_cffi/
%{_libdir}/python%{py3_ver}/site-packages/zm-proto_cffi-*-py%{py3_ver}.egg-info/
%endif

%prep
#FIXME: %{error:...} did not worked for me
%if %{with python_cffi}
%if %{without drafts}
echo "FATAL: python_cffi not yet supported w/o drafts"
exit 1
%endif
%endif

%setup -q

%build
sh autogen.sh
%{configure} --enable-drafts=%{DRAFTS}
make %{_smp_mflags}

%if %{with python_cffi}
# Problem: we need pkg-config points to built and not yet installed copy of zm-proto
# Solution: chicken-egg problem - let's make "fake" pkg-config file
sed -e "s@^libdir.*@libdir=`pwd`/src/.libs@" \
    -e "s@^includedir.*@includedir=`pwd`/include@" \
    src/libzm_proto.pc > bindings/python_cffi/libzm_proto.pc
cd bindings/python_cffi
export PKG_CONFIG_PATH=`pwd`
python2 setup.py build
python3 setup.py build
%endif

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

%if %{with python_cffi}
cd bindings/python_cffi
export PKG_CONFIG_PATH=`pwd`
python2 setup.py install --root=%{buildroot} --skip-build --prefix %{_prefix}
python3 setup.py install --root=%{buildroot} --skip-build --prefix %{_prefix}
%endif

%files
%defattr(-,root,root)
%{_bindir}/zmpub
%{_mandir}/man1/zmpub*
%{_bindir}/zmsub
%{_mandir}/man1/zmsub*

%changelog
