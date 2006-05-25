%define binary_version 1.9

Name:		 cr
Summary:	 Chromium
Version:	 1.9
Release:	 1
License:	 BSD
Group:		 User Interface/X
Source:		 %{name}-%{version}.tar.bz2
BuildRoot:	 %{_tmppath}/%{name}.%{version}-buildroot
URL:		 http://chromium.sourceforge.net

%description

%prep
%setup -q -n cr-%{version}

%build
make

%install
rm -rf $RPM_BUILD_ROOT

install -d $RPM_BUILD_ROOT/usr/local/cr-%{version}/bin \
           $RPM_BUILD_ROOT/usr/local/cr-%{version}/lib \
           $RPM_BUILD_ROOT/usr/local/cr-%{version}/include \
           $RPM_BUILD_ROOT/usr/local/cr-%{version}/include/state \
           $RPM_BUILD_ROOT/usr/local/share/cr-%{version} \
	   $RPM_BUILD_ROOT/usr/local/share/doc/cr-%{version}/doc \
           $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership \
           $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/configs \
	   $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/prshd \
	   $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/server \
	   $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/tools

install bin/Linux/* \
	$RPM_BUILD_ROOT/usr/local/cr-%{version}/bin

install lib/Linux/*.a \
	$RPM_BUILD_ROOT/usr/local/cr-%{version}/lib

install lib/Linux/*.so \
	$RPM_BUILD_ROOT/usr/local/cr-%{version}/lib

#install -m 644 doc/* \
#               $RPM_BUILD_ROOT/usr/local/share/doc/cr-%{version}/doc
#install -m 644 doxygen/* \
#               $RPM_BUILD_ROOT/usr/local/share/doc/cr-%{version}/html
install -m 644 include/*.h \
               $RPM_BUILD_ROOT/usr/local/cr-%{version}/include
install -m 644 include/state/*.h \
               $RPM_BUILD_ROOT/usr/local/cr-%{version}/include/state
install -m 644 mothership/configs/*.conf \
               $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/configs
install -m 644 mothership/prshd/*.py \
               $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/prshd
install -m 644 mothership/server/*.py \
               $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/server
install -m 644 mothership/tools/*.py \
               $RPM_BUILD_ROOT/usr/local/share/cr-%{version}/mothership/tools

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc LICENSE TO-DO doc/* COPYRIGHT.LLNL COPYRIGHT.REDHAT

/usr/local/cr-%{version}/bin/*
/usr/local/cr-%{version}/include/*.h
/usr/local/cr-%{version}/include/state/*.h
/usr/local/cr-%{version}/lib/*.so
/usr/local/cr-%{version}/lib/*.a
/usr/local/share/cr-%{version}/mothership/prshd/prshd.py
/usr/local/share/cr-%{version}/mothership/server/crconfig.py
/usr/local/share/cr-%{version}/mothership/server/crmatrix.py
/usr/local/share/cr-%{version}/mothership/server/daughtership.py
/usr/local/share/cr-%{version}/mothership/server/mothership.py
/usr/local/share/cr-%{version}/mothership/server/tilelayout.py
/usr/local/share/cr-%{version}/mothership/tools/*.py
/usr/local/share/cr-%{version}/mothership/configs/*.conf

%changelog
* Thu May 27 2004 Alan Matsuoka <alanm@redhat.com>
- Initial version
