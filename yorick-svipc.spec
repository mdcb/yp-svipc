%define _prefix __auto__
%define version __auto__
%define release __auto__

%undefine _prefix
%define _prefix /usr
%undefine version
%define version 0.3
%undefine release
%define release 1

Name: yorick-svipc
Summary: System V IPC for Yorick
Version: %{version}
Release: 1%{?dist}.gemini
License: mdcb808@gmail.com
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Source0: %{name}-%{version}.tar.gz
BuildRequires: yorick
Requires: yorick

%description
System V IPC for Yorick

%package -n python-svipc
Summary: System V IPC for Python
%description -n python-svipc
System V IPC for Python
BuildRequires: python
Requires: python

%define debug_package %{nil}
%define y_exe_site %(echo Y_SITE  | yorick -q | awk -F '"' '{print $2}')
%define python_lib %(python -c "from  distutils import sysconfig;print sysconfig.get_python_lib()")

%description
Sys V IPC wrapper for yorick.

%prep
%setup -q -n %name

%build
( cd yorick && yorick -batch make.i && gmake clean all )
( cd python && python setup.py build )

%install
( cd yorick && gmake install DESTDIR=$RPM_BUILD_ROOT )
( cd python && python setup.py install --root $RPM_BUILD_ROOT --prefix %_prefix )

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%y_exe_site/*

%files -n python-svipc
%defattr(-,root,root,-)
%python_lib/*

%changelog
* Tue Feb 23 2010 Matthieu Bec <mbec@gemini.edu> 0.3-1
- multi spec file
* Mon Feb 15 2010 Matthieu Bec <mbec@gemini.edu> 0.1-3
- first spec
