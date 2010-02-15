%define _prefix __auto__
%define version __auto__
%define release __auto__

Summary: System V IPC for Yorick
Name: yorick-svipc
Version: %{version}
Release: 1%{?dist}.gemini
URL: http://www.gemini.edu
Packager: Matthieu Bec <mbec@gemini.edu>
License: mbec@gemini.edu
Group: Gemini
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: yorick
Requires: yorick

Source0: %{name}-%{version}.tar.gz

%define debug_package %{nil}
%define y_exe_home %(echo Y_HOME  | yorick -q | awk -F '"' '{print $2}')
%define y_exe_site %(echo Y_SITE  | yorick -q | awk -F '"' '{print $2}')

%description
Sys V IPC wrapper for yorick.

%prep
%setup -q -n %name

%build
gmake -f yorick-svipc.spec
gmake

%install
mkdir -p  $RPM_BUILD_ROOT/%y_exe_home/lib $RPM_BUILD_ROOT/%y_exe_site/i-start $RPM_BUILD_ROOT/%y_exe_site/i0
cp svipc.so $RPM_BUILD_ROOT/%y_exe_home/lib
#cp svipc_start.i $RPM_BUILD_ROOT/%y_exe_site/i-start
cp svipc.i $RPM_BUILD_ROOT/%y_exe_site/i0

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%y_exe_home/lib/svipc.so
#%y_exe_site/i-start/svipc_start.i
%y_exe_site/i0/svipc.i

%changelog
* Mon Feb 15 2010 Matthieu Bec <mbec@gemini.edu> 0.1-2
- first spec
