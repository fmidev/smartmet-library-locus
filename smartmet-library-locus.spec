%define DIRNAME locus
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}
Summary: locus library
Name: %{SPECNAME}
Version: 17.4.26
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-locus
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: boost-devel
BuildRequires: libpqxx-devel >= 4.0.1
BuildRequires: postgresql93-devel
BuildRequires: smartmet-library-macgyver-devel >= 17.3.16
Requires: smartmet-library-macgyver >= 17.3.16
Requires: libpqxx >= 4.0.1
Requires: postgresql93-libs
Provides: %{SPECNAME}
Obsoletes: libsmartmet-locus < 16.12.20
Obsoletes: libsmartmet-locus-debuginfo < 16.12.20


%description
FMI locus library

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}
 
%build
make %{_smp_mflags}

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0775,root,root,0775)
%{_libdir}/libsmartmet-%{DIRNAME}.so

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%package -n %{SPECNAME}-devel
Summary: FMI Locus library development files
Provides: %{SPECNAME}-devel
Obsoletes: libsmartmet-locus-devel  < 16.12.20

%description -n %{SPECNAME}-devel
FMI Locus library development files

%files -n %{SPECNAME}-devel
%defattr(0664,root,root,0775)
%{_includedir}/smartmet/%{DIRNAME}

%changelog
* Wed Apr 26 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.26-1.fmi
- Fixed name search hash value to include the list of allowed keywords

* Thu Mar 23 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.23-1.fmi
- Force user to specify the host, username, password and database.

* Tue Mar 14 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.14-1.fmi
- Switched to using macgyver StringConversion tools

* Fri Feb  3 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.3-1.fmi
- Added possibility to select the PostgreSQL port number

* Tue Dec 20 2016 Mika Heiskanen <mika+.heiskanen@fmi.fi> - 16.12.20-1.fmi
- Switched to using open source naming conventions

* Tue Jun  7 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.7-1.fmi
- Use gemini by default instead of popper

* Mon Apr 11 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.4.11-1.fmi
- Added QueryOptions::HashValue(), marked Hash() as deprecated

* Sun Feb  7 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.2.7-1.fmi
- Switched from scons to normal make
- Fixed debuginfo packaging

* Thu Nov 12 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.12-1.fmi
- Removed explicit use of std streams completely

* Wed Nov 11 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.11-1.fmi
- Reduced the use of std streams, to_lower and to_upper calls to avoid global std::locale locks

* Wed Aug 26 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.8.26-1.fmi
- Input string quoting was broken, now using pqxx quote

* Tue Jun 23 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.6.23-1.fmi
- Make sure the alternate name is always chosen the same way by using name sort as the last resort

* Mon Apr 27 2015 Anssi Reponen <anssi.reponen@fmi.fi> - 15.4.27-1.fmi
- Adding null value check for fields 'iso2', 'features_code', 'municipalities_id'

* Thu Apr 9 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.4.9-1.fmi
- Country name resolving now obeys preferred and priority - fields

* Wed Aug 27 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.27-1.fmi
- Use ADM2 instead of ADM3 - geonames changed the classifications in Finland

* Thu May 15 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.15-1.fmi
- Drop KNN safety limit from 50 to 10 to increase query speed significantly

* Wed May 14 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.14-1.fmi
- Build a shared library
- Omit DWithin from latlon searches, since psql 9.3 is faster without it and with KNN indexes

* Fri May  2 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.2-1.fmi
- Use <-> operator for ordering latlon searches, it is fast since PostgreSQL 9.1

* Mon Apr 28 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.28-1.fmi
- Rebuild with static postgresql 9.3 linkage

* Thu Jan  9 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.1.9-1.fmi
- Fixed keyword searches to return all locations instead of max 100

* Wed Dec  4 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.12.4-1.fmi
- Revamped the API to be less confusing

* Mon Dec  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.12.2-1.fmi
- Disable historic alternate names as translations

* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-2.fmi
- Remove locations in name searches without a valid timezone

* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-1.fmi
- Ignore locations without a valid timezone

* Tue Nov 26 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.26-1.fmi
- Fixed name comparisons to be case insensitive

* Mon Nov 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.25-1.fmi
- Location builder now caches subquery results to minimize database queries

* Fri Nov 22 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.22-1.fmi
- Fixed geoid and elevation to be signed integers

* Tue Nov 19 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.19-1.fmi
- Fixed the Query constructor to use the given user name, password etc

* Thu Nov 14 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.14-1.fmi
- Fixed latlon searches to use PostgreSQL 2D indexes
- Fixed keyword searches

* Tue Nov 12 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.12-1.fmi
- Fixed linkage to libpqxx
- Fixed sorting of alternate names based on length
- Sort keyword fetch by name

* Mon Nov 11 2013 Anssi Reponen <anssi.reponen@fmi.fi> - 13.11.11-1.fmi
- FetchById-function corrected: if the initial query didnt give any results and id is > 10000000 we execute another guery with negative id (id*-1)

* Tue Oct 08 2013 Anssi Reponen <anssi.reponen@fmi.fi> - 13.10.08-1.fmi
- Initial PosrgreSQL version of fminames library. Based on MySQL version. Library name changed to locus.