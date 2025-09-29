%define DIRNAME locus
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}
Summary: locus library
Name: %{SPECNAME}
Version: 25.9.29
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-locus
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

%if 0%{?rhel} && 0%{rhel} <= 9
%define smartmet_fmt_min 11.0.1
%define smartmet_fmt_max 12.0.0
%define smartmet_fmt fmt-libs >= %{smartmet_fmt_min}, fmt-libs < %{smartmet_fmt_max}
%define smartmet_fmt_devel fmt-devel >= %{smartmet_fmt_min}, fmt-devel < %{smartmet_fmt_max}
%else
%define smartmet_fmt fmt
%define smartmet_fmt_devel fmt-devel
%endif

BuildRequires: %{smartmet_boost}-devel
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: postgresql15-devel
BuildRequires: %{smartmet_fmt_devel}
BuildRequires: rpm-build
BuildRequires: smartmet-library-macgyver-devel >= 25.9.19
Requires: %{smartmet_boost}-locale
Requires: %{smartmet_boost}-regex
Requires: %{smartmet_boost}-system
Requires: %{smartmet_boost}-thread
Requires: smartmet-library-macgyver >= 25.9.19

%if 0%{?rhel} && 0%{rhel} == 8
Requires: libpqxx >= 1:7.7.0, libpqxx < 1:7.8.0
BuildRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
#TestRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
%else
%if 0%{?rhel} && 0%{rhel} == 9
Requires: libpqxx >= 1:7.9.0, libpqxx < 1:7.10.0
BuildRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
#TestRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
%else
%if 0%{?rhel} && 0%{rhel} >= 10
Requires: libpqxx >= 1:7.10.0, libpqxx < 1:7.11.0
BuildRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
#TestRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
%else
Requires: libpqxx
BuildRequires: libpqxx-devel
#TestRequires: libpqxx-devel
%endif
%endif
%endif

#TestRequires: %{smartmet_boost}-devel
#TestRequires: gcc-c++
#TestRequires: make
#TestRequires: postgresql15-libs
#TestRequires: smartmet-library-macgyver-devel >= 25.9.19
#TestRequires: smartmet-library-regression
#TestRequires: smartmet-test-db >= 25.6.18
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
Requires: %{SPECNAME}
Requires: libpqxx-devel
Obsoletes: libsmartmet-locus-devel  < 16.12.20

%description -n %{SPECNAME}-devel
FMI Locus library development files
Requires: %{SPECNAME} = %version-%release
Requires: smartmet-library-macgyver-devel

%files -n %{SPECNAME}-devel
%defattr(0664,root,root,0775)
%{_includedir}/smartmet/%{DIRNAME}

%changelog
* Mon Sep 29 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.9.29-1.fmi
- Fixed compiler warnings

* Wed Jun  4 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.6.4-2.fmi
- Additional database use optimisation

* Wed Jun  4 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.6.4-1.fmi
- Database use optimisation

* Tue Feb 18 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.18-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Sat Sep 28 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.9.28-1.fmi
- Repackaged due to PostgreSQLConnection ABI change

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Mon Jul 10 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.10-1.fmi
- Use postgresql 15, gdal 3.5, geos 3.11 and proj-9.0

* Tue Mar  7 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.3.7-1.fmi
- Silenced CodeChecker warnings

* Mon Feb 27 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.2.27-1.fmi
- Query: added method cancel() to cancel database operation (or reconnection attempts)

* Fri Feb 24 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.2.24-1.fmi
- Fix SQL errors with PostGIS 3.2.4

* Fri Dec 16 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.12.16-1.fmi
- Repackaged since PostgreSQLConnection ABI changed

* Fri Jun 17 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.17-1.fmi
- Add support for RHEL9. Update libpqxx to 7.7.0 (rhel8+) and fmt to 8.1.1

* Tue Jun  7 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.7-1.fmi
- Do not return colloquial alternate names

* Mon Mar 28 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.3.28-1.fmi
- Repackage due to smartmet-library-macgyver changes

* Mon Jan 31 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.31-1.fmi
- Use language codes table from geonames database if loaded

* Tue Dec  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.12.7-1.fmi
- Update to postgresql 13 and gdal 3.3

* Wed Aug 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.11-1.fmi
- Avoid boost tolower functions for using std::locale

* Thu Jul  8 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.7.8-1.fmi
- Use libpqxx7 for RHEL8

* Mon Jun 21 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.6.21-1.fmi
- Use Fmi::Database::PostgreSQLConnection instead of own connection class

* Fri Jun 18 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.18-2.fmi
- Silenced CodeChecker warnings

* Wed Jun 16 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.16-1.fmi
- Use Fmi::Exception

* Thu May 20 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.20-2.fmi
- Repackaged with improved hashing functions

* Thu May 20 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.20-1.fmi
- Use Fmi hash functions, boost::combine produces too many collisions

* Thu Feb 18 2021 Anssi Reponen <anssi.reponen@fmi.fi> - 21.2.18-1.fmi
- Added name_type in QueryOptions, so that there is no more need to store fmisid-,wmo-,lpnn-id in 
language-field in FetchByName-query(BRAINSTORM-1848)

* Tue Feb  2 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.2-1.fmi
- Fixed lower case conversions to use Boost.Locale

* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues

* Tue Jan  5 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.5-1.fmi
- Repackaged with latest postgresql packages

* Tue Dec 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.15-1.fmi
- Upgrade to pgdg12

* Thu Dec  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.3-1.fmi
- Silenced CodeChecker warnings

* Wed Oct  7 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.7-1.fmi
- Build update: use makefile.inc from smartmet-library-macgyver
- Fail build in case of unresolvedreferences in built shared library

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Wed Jun 10 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.10-1.fmi
- Require postgresql from PGDG instead of EPEL

* Mon Jun  8 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.8-1.fmi
- Upgraded libpqxx

* Wed May 20 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.5.20-1.fmi
- Fixed the default feature list to use AIRP (airport) instead of AIRB (airbase)

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgrade to Boost 1.69

* Wed Dec  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.4-1.fmi
- Use -fno-omit-frame-pointer for a better profiling and debugging experience

* Wed Aug 28 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.28-1.fmi
- Added optional fmisid information for locations

* Thu Mar 21 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.21-1.fmi
- Added mountains, parks, airports and harbours to default coordinate search feature list

* Fri Nov 16 2018 Heikki Pernu <heikki.pernu@fmi.fi> - 18.11.16-1.fmi
- Remove Postgresql 9.5 library dependencies

* Tue Aug 21 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.21-1.fmi
- Silenced CodeChecker warnings

* Mon Aug  6 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.6-1.fmi
- Silenced CodeChecker warnings

* Wed Aug  1 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.1-1.fmi
- Use C++11 for-loops instead of BOOST_FOREACH

* Thu Jun 14 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.14-1.fmi
- Improved error message on connection failure

* Wed Jun 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.13-1.fmi
- Transaction-functions added

* Mon May 21 2018 Anssi Reponen <anssi.reponen@fmi.fi> - 18.5.21-1.fmi
- New constructor added to allow connect_timeout parameter for PostgreSQL connection

* Sat Apr  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.7-1.fmi
- Upgrade to boost 1.66

* Mon Jan 15 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.1.15-1.fmi
- Updated libpqxx and postgresql dependencies

* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65

* Thu May 18 2017 Pertti Kinnia <pertti.kinnia@fmi.fi> - 17.5.18-1.fmi
- Added 'ansiname' column null check 

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
