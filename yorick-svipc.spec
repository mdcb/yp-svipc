%define _prefix __auto__
#%define version __auto__
%define release __auto__

%undefine _prefix
%define _prefix /usr
%undefine version
%define version 0.4
%undefine release
%define release 2

Name: plugin-svipc
Summary: System V IPC plugins for Python and Yorick
Version: %{version}
Release: 1%{?dist}.gemini
License: mdcb808@gmail.com
Group: Gemini
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Source0: %{name}-%{version}.tar.gz
BuildRequires: python, yorick

%package -n python-svipc
Summary: System V IPC plugin for Python
Group: Gemini
%description -n python-svipc
System V IPC wrapper for Python
Requires: python

%package -n yorick-svipc
Summary: System V IPC plugin for Yorick
Group: Gemini
%description -n yorick-svipc
System V IPC wrapper for Yorick
Requires: yorick

%define debug_package %{nil}
%define y_site %(echo Y_SITE  | yorick -q | awk -F '"' '{print $2}')
%define python_lib %(python -c "from  distutils import sysconfig;print sysconfig.get_python_lib()")

%description
Sys V IPC wrappers for Python and Yorick 

%prep
#%setup -q -n %name
%setup -q -n yorick-svipc

%build
( cd yorick && yorick -batch make.i && gmake clean all )
( cd python && python setup.py build )

%install
( cd yorick && gmake install DESTDIR=$RPM_BUILD_ROOT )
( cd python && python setup.py install --root $RPM_BUILD_ROOT --prefix %_prefix )

%clean
rm -rf $RPM_BUILD_ROOT

%files -n yorick-svipc
%defattr(-,root,root,-)
%y_site/*

%files -n python-svipc
%defattr(-,root,root,-)
%python_lib/*

%changelog
* Wed Apr 7 2010 Matthieu Bec <mbec@gemini.edu> 0.4-2
- see ChangeLog
* Tue Feb 23 2010 Matthieu Bec <mbec@gemini.edu> 0.3-1
- multi spec file
* Mon Feb 15 2010 Matthieu Bec <mbec@gemini.edu> 0.1-3
- first spec
