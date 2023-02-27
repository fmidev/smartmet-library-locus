#include "Query.h"
#include "QueryOptions.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <macgyver/PostgreSQLConnection.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace boost;
using namespace Locus;

#ifndef DATABASE_HOST
#define DATABASE_HOST "smartmet-test"
#endif
#ifndef DATABASE_USER
#define DATABASE_USER "fminames_user"
#endif
#ifndef DATABASE_PASS
#define DATABASE_PASS "fminames_pw"
#endif
#ifndef DATABASE_PORT
#define DATABASE_PORT "5444"
#endif
#ifndef DATABASE
#define DATABASE "fminames"
#endif

// Some helpful debugging functions

ostream& operator<<(ostream& out, const Locus::SimpleLocation& theLoc)
{
  out << "Name\t= '" << theLoc.name << "'\n"
      << "Lat\t= " << theLoc.lat << "\n"
      << "Lon\t= " << theLoc.lon << "\n"
      << "Country\t= '" << theLoc.country << "'\n"
      << "Feature\t= '" << theLoc.feature << "'\n"
      << "Description\t= '" << theLoc.description << "'\n"
      << "Admin\t= '" << theLoc.admin << "'\n"
      << "Timezone\t= '" << theLoc.timezone << "'\n"
      << "Population\t= '" << theLoc.population << "'\n"
      << "ISO2\t= '" << theLoc.iso2 << "'\n"
      << "ID\t= '" << theLoc.id << "'\n"
      << "Elevation\t= '" << theLoc.elevation << "'\n";
  return out;
}

ostream& operator<<(ostream& out, const Locus::Query::return_type& theLocs)
{
  int i = 1;
  for (Locus::Query::return_type::const_iterator it = theLocs.begin(); it != theLocs.end(); ++it)
  {
    out << "Location " << i << ":" << endl << *it << endl;
    ++i;
  }
  return out;
}

namespace QueryTest
{
// ----------------------------------------------------------------------

void name_id_search()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Locus::QueryOptions opts;
  opts.SetCountries("all");
  opts.SetSearchVariants(true);
  opts.SetResultLimit(1);
  Query::return_type ret;

  // FMISID
  opts.SetNameType("fmisid");
  opts.SetLanguage("fi");
  ret = lq.FetchByName(opts, "101004");

  if (ret.size() > 0)
    if (ret[0].name != "Kumpula")
      TEST_FAILED("Name of FMISID 101004 should be Kumpula, not " + ret[0].name);

  opts.SetLanguage("sv");
  ret = lq.FetchByName(opts, "101004");

  if (ret.size() > 0)
    if (ret[0].name != "Gumtäkt")
      TEST_FAILED("Name of FMISID 101004 should be Gumtäkt, not " + ret[0].name);

  // WMO
  opts.SetNameType("wmo");
  opts.SetLanguage("fi");
  ret = lq.FetchByName(opts, "2998");

  if (ret.size() > 0)
    if (ret[0].name != "Kumpula")
      TEST_FAILED("Name of WMO 2998 should be Kumpula, not " + ret[0].name);

  opts.SetLanguage("sv");
  ret = lq.FetchByName(opts, "2998");

  if (ret.size() > 0)
    if (ret[0].name != "Gumtäkt")
      TEST_FAILED("Name of WMO 2998 should be Gumtäkt, not " + ret[0].name);

  // LPNN
  opts.SetNameType("lpnn");
  opts.SetLanguage("fi");
  ret = lq.FetchByName(opts, "339");

  if (ret.size() > 0)
    if (ret[0].name != "Kumpula")
      TEST_FAILED("Name of LPNN 339 should be Kumpula, not " + ret[0].name);

  opts.SetLanguage("sv");
  ret = lq.FetchByName(opts, "339");

  if (ret.size() > 0)
    if (ret[0].name != "Gumtäkt")
      TEST_FAILED("Name of LPNN 339 should be Gumtäkt, not " + ret[0].name);

  TEST_PASSED();
}

void simple_name_search()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;

  ret = lq.FetchByName(options, "Helsinki");

  if (ret.size() != 1)
    TEST_FAILED("Should find 1 match for Helsinki");

  if (ret[0].name != "Helsinki")
    TEST_FAILED("Name of first match should be Helsinki, not " + ret[0].name);
  if (ret[0].country != "Suomi")
    TEST_FAILED("Country of first match should be Suomi, not " + ret[0].country);
  if (ret[0].timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of first match should be Europe/Helsinki, not " + ret[0].timezone);

  if (ret[0].id != 658225)
    TEST_FAILED("ID of Helsinki should be 658225, not " +
                boost::lexical_cast<std::string>(ret[0].id));
  if (ret[0].elevation != 10)
    TEST_FAILED("Elevation of Helsinki should be 10, not " +
                boost::lexical_cast<std::string>(ret[0].elevation));

  options.SetCountries("all");
  options.SetLanguage("%");
  ret = lq.FetchByName(options, "New York");

  if (ret.size() != 1)
    TEST_FAILED("Should find 1 match for New York, not " +
                boost::lexical_cast<std::string>(ret.size()));

  options.SetLanguage("fi");
  ret = lq.FetchByName(options, "Stockholm");

  if (ret[0].country != "Ruotsi")
    TEST_FAILED("Stockholm should be located in Ruotsi, not in " + ret[0].country);
  if (ret[0].feature != "PPLC")
    TEST_FAILED("Stockholm should be capital (PPLC)");

  // Must not find the historic Hievala name
  ret = lq.FetchByName(options, "Evitskog");
  if (ret[0].name != "Evitskog")
    TEST_FAILED("Should find Evitskog, not " + ret[0].name);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void utf8_region_search()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetLanguage("sv");

  ret = lq.FetchByName(options, "åbo,åbo");
  if (ret.size() < 1)
    TEST_FAILED("Should find at least 1 match for åbo,åbo, found zero");
  if (ret[0].name != "Åbo")
    TEST_FAILED("Name of first match should be Åbo, not " + ret[0].name);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void search_with_area()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;

  ret = lq.FetchByName(options, "Kumpula,Helsinki");
  if (ret.size() != 1)
    TEST_FAILED("Should find 1 Kumpula,Helsinki, not " + lexical_cast<string>(ret.size()));

  ret = lq.FetchByName(options, "Kumpula,Suomi");
  if (ret.size() != 8)
    TEST_FAILED("Should find 8 Kumpula,Suomi locations, not " +
                boost::lexical_cast<std::string>(ret.size()));

  options.SetCountries("fi");
  options.SetLanguage("en");
  ret = lq.FetchByName(options, "Kumpula,Helsinki");
  if (ret.size() != 1)
    TEST_FAILED("Should find 1 Kumpula,Helsinki (lang=en), not " +
                lexical_cast<string>(ret.size()));

  options.SetCountries("all");
  ret = lq.FetchByName(options, "Bago");
  if (ret.empty())
    TEST_FAILED("Should find 1 Bago (lang=en), not 0");

  if (ret[0].country != "Myanmar [Burma]")
    TEST_FAILED("Incorrect country for Bago (lang=en): '" + ret[0].country + "'")

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void escape()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;

  try
  {
    ret = lq.FetchByName(options, "Kumpula;");
  }
  catch (std::exception& err)
  {
    std::cout << "Exception string: " << err.what() << std::endl;
    TEST_FAILED("Caught an exception when handling special character \";\"");
  }

  try
  {
    ret = lq.FetchByName(options, "Kumpula; SELECT foo foo");
  }
  catch (std::exception& err)
  {
    std::cout << "Exception string: " << err.what() << std::endl;
    TEST_FAILED("Caught an exception when injecting query");
  }

  try
  {
    ret = lq.FetchByName(options, "Kumpula'");
  }
  catch (std::exception& err)
  {
    std::cout << "Exception string: " << err.what() << std::endl;
    TEST_FAILED("Caught an exception when handling special character \"'\"");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void search_id()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;

  ret = lq.FetchById(options, 658225);

  if (ret.size() != 1)
    TEST_FAILED("Should find Helsinki PPLC");

  if (ret[0].name != "Helsinki")
    TEST_FAILED("Name of first match should be Helsinki");
  if (ret[0].country != "Suomi")
    TEST_FAILED("Country of first match should be Helsinki");
  if (ret[0].timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of first match should be Europe/Helsinki");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void multiple_matches()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetSearchVariants(false);

  ret = lq.FetchByName(options, "Koski");

  if (ret.size() != 12)
    TEST_FAILED("Should find Koski 12 times, not " + lexical_cast<string>(ret.size()));

  if (ret[0].name != "Koski")
    TEST_FAILED("Name of first match should be Koski, not " + ret[0].name);
  if (ret[0].country != "Suomi")
    TEST_FAILED("Country of first match should be Suomi, not " + ret[0].country);
  if (ret[0].timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of first match should be Europe/Helsinki, not " + ret[0].timezone);

  if (ret.back().name != "Koski")
    TEST_FAILED("Name of last match should be Koski, not " + ret.back().name);
  if (ret.back().country != "Suomi")
    TEST_FAILED("Country of last match should be Suomi, not " + ret.back().country);
  if (ret.back().timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of last match should be Europe/Helsinki, not " + ret.back().timezone);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_variants()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetSearchVariants(true);
  // options.SetCharset("latin1");

  ret = lq.FetchByName(options, "Koski");

  if (ret.size() != 9)
    TEST_FAILED("Should find Koski 9 times, not " + boost::lexical_cast<string>(ret.size()) +
                " times");

  if (ret[0].name != "Koski Tl")
    TEST_FAILED("Name of first match should be Koski, not " + ret[0].name.substr(0, 9));
  if (ret[0].country != "Suomi")
    TEST_FAILED("Country of first match should be Koski");
  if (ret[0].timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of first match should be Europe/Helsinki");

  if (ret.back().name != "Koski")
    TEST_FAILED("Name of last match should be Koski");
  if (ret.back().country != "Suomi")
    TEST_FAILED("Country of last match should be Suomi");
  if (ret.back().timezone != "Europe/Helsinki")
    TEST_FAILED("Timezone of last match should be Europe/Helsinki");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_countries()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetCountries("cz,sk");
  options.SetSearchVariants(false);

  ret = lq.FetchByName(options, "Prague");

  if (ret.size() != 1)
    TEST_FAILED("Should find Praha 1 time, not " + lexical_cast<string>(ret.size()));

  if (ret[0].name != "Praha")
    TEST_FAILED("Name of first match should be Praha");

  if (ret[0].timezone != "Europe/Prague")
    TEST_FAILED("Timezone of first match should be Europe/Prague, not " + ret[0].timezone);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_language()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetCountries("cz,sk");
  options.SetLanguage("sv");

  ret = lq.FetchByName(options, "Prague");
  if (ret.size() != 1)
    TEST_FAILED("Should find Prague 1 time");
  if (ret[0].name != "Prag")
    TEST_FAILED("Name of first match should be Prag, not " + ret[0].name);
  if (ret[0].timezone != "Europe/Prague")
    TEST_FAILED("Timezone of first match should be Europe/Prague");

  options.SetCountries("fi");
  ret = lq.FetchByName(options, "Helsinki");
  if (ret.size() != 1)
    TEST_FAILED("Should find Helsinki 1 time");
  if (ret[0].name != "Helsingfors")
    TEST_FAILED("Name of first match should be Helsingfors, not " + ret[0].name)
  if (ret[0].country != "Finland")
    TEST_FAILED("Country of first match should be Finland");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_result_limit()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  QueryOptions options;
  options.SetResultLimit(5);

  Query::return_type ret = lq.FetchByName(options, "Kumpula");

  if (ret.size() != 5)
    TEST_FAILED("Should find Kumpula 5 times, not " + lexical_cast<string>(ret.size()));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_features()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);
  Query::return_type ret;

  QueryOptions options;
  options.SetCountries("%");
  options.SetFeatures("PPLC");
  ret = lq.FetchByName(options, "Praha");

  if (ret.size() != 1)
    TEST_FAILED("Should find Praha 1 time");

  if (ret[0].name != "Praha")
    TEST_FAILED("Name of first match should be Praha");
  if (ret[0].feature != "PPLC")
    TEST_FAILED("First match should be the capital");

  TEST_PASSED();
}

void specific_keywords()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;
  options.SetCountries("all");
  options.SetFeatures("all");
  options.SetKeywords("finavia");

  ret = lq.FetchByName(options, "Kemi");

  if (ret.size() != 1)
    TEST_FAILED("Should find 1 Kemi with keyword 'finavia' and name search, not " +
                lexical_cast<string>(ret.size()));

  ret = lq.FetchByLatLon(options, 62.6, 29.7667, 30);

  if (ret.size() != 1)
    TEST_FAILED("Should find 1 Joensuu airport with keyword 'finavia' and coordinate search, not " +
                lexical_cast<string>(ret.size()));

  if (ret[0].name != "Joensuun lentoasema")
    TEST_FAILED("Failed to find Joensuu airport with a coordinate search with keyword 'finavia'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void simple_lonlat_search()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;
  options.SetResultLimit(2);
  ret = lq.FetchByLonLat(options, 24.97, 60.10);

  if (ret.size() != 2)
    TEST_FAILED("Should find 2 matches since limit is 2");

  if (ret[0].name != "Harmaja")
    TEST_FAILED("Name of first match should be Harmaja");

  if (ret.back().name != "Kuivasaari")
    TEST_FAILED("Name of second match should be Kuivasaari, not " + ret.back().name);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void simple_latlon_search()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;
  options.SetResultLimit(2);
  ret = lq.FetchByLatLon(options, 60.10, 24.97);

  if (ret.size() != 2)
    TEST_FAILED("Should find 2 matches since limit is 2");

  if (ret[0].name != "Harmaja")
    TEST_FAILED("Name of first match should be Harmaja, not " + ret[0].name);

  if (ret.back().name != "Kuivasaari")
    TEST_FAILED("Name of second match should be Kuivasaari, not " + ret.back().name);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void specific_radius()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;
  ret = lq.FetchByLonLat(options, 24.97, 60.10, 4);

  if (ret.size() < 10)
    TEST_FAILED("Should find atleast 10 matches in radius 4km");

  if (ret[0].name != "Harmaja")
    TEST_FAILED("Name of first match should be Harmaja");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

void search_keyword()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;

  ret = lq.FetchByKeyword(options, "municipalities_fi");
  if (ret.size() < 300)
    TEST_FAILED("Should find > 300 locations for keyword 'municipalities_fi', found " +
                boost::lexical_cast<string>(ret.size()));

  for (unsigned int i = 0; i < ret.size(); i++)
    if (ret[i].name == "NULL")
      TEST_FAILED("Municipality name is NULL for index " + boost::lexical_cast<string>(i));

  TEST_PASSED();
}

void count_keywords()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;

  unsigned int count = lq.CountKeywordLocations(options, "finavia");

  if (count != 24)
    TEST_FAILED("Expected 24 locations for keyword 'finavia', got " +
                boost::lexical_cast<string>(count));

  TEST_PASSED();
}

void search_in_autocompletemode()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;

  options.SetResultLimit(20);
  ret = lq.FetchByName(options, "Ii%");

  if (ret.size() == 0)
    TEST_FAILED("Found 0 results for autocomplete search Ii%");

  // Iisalmi has ~22000 people, Ii only 9300
  if (ret[0].name != "Iisalmi")
    TEST_FAILED("Should find Iisalmi first if autocomplete mode is off, not " + ret[0].name);

  options.SetAutocompleteMode(true);

  ret = lq.FetchByName(options, "Ii%");

  if (ret[0].name != "Ii")
    TEST_FAILED("Should find Ii first if autocomplete mode is on");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void latin1()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  Query::return_type ret;

  QueryOptions options;
  // options.SetCharset("LATIN1");	// case does not matter

  ret = lq.FetchById(options, 745044);

  if (ret.size() != 1)
    TEST_FAILED("Should find Istanbul");

  if (ret[0].name != "Istanbul")
    TEST_FAILED("Name of first match should be Istanbul in Latin1");

  TEST_PASSED();
}

#ifdef TEST_PRIVATE

void resolve_feature()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  lq.SetDebug(true);

  QueryOptions options;

  string desc = lq.ResolveFeature(options, "PCLD");

  if (desc != "dependent political entity")
    TEST_FAILED("Description for PCLF should be: 'dependent political entity', NOT '" + desc + "'");

  TEST_PASSED();
}

void resolve_name_variant()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  lq.SetDebug(true);

  QueryOptions options;
  options.SetLanguage("");

  string name = lq.ResolveNameVariant(options, "13");

  if (name != "Takht Arreh Yek")
    TEST_FAILED("Name for id 13 should be: 'Takht Arreh Yek', NOT '" + name + "'");

  TEST_PASSED();
}

void resolve_country()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  lq.SetDebug(true);

  QueryOptions options;
  options.SetLanguage("");

  string country = lq.ResolveCountry(options, "FI");

  if (country != "Finland")
    TEST_FAILED("Name for id 13 should be: 'Finland', NOT '" + country + "'");

  TEST_PASSED();
}

void resolve_municipality()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  lq.SetDebug(true);

  QueryOptions options;
  options.SetLanguage("fi");

  string municipality = lq.ResolveMunicipality(options, "5");

  if (municipality != "Artjärvi")
    TEST_FAILED("Name for id 13 should be: 'Artjärvi', NOT '" + municipality + "'");

  options.SetLanguage("sv");

  municipality = lq.ResolveMunicipality(options, "5");

  if (municipality != "Artsjö")
    TEST_FAILED("Name for id 13 should be: 'Artsjö', NOT '" + municipality + "'");

  TEST_PASSED();
}

void resolve_administrative()
{
  Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

  lq.SetDebug(true);

  string admin = lq.ResolveAdministrative("13", "DE");

  if (admin != "Saxony")
    TEST_FAILED("Name for id 13 should be: 'Saxony', NOT '" + admin + "'");

  TEST_PASSED();
}

#endif

// ----------------------------------------------------------------------

// The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(search_keyword);
    TEST(count_keywords);
    TEST(latin1);
    TEST(escape);
    TEST(search_id);
    TEST(search_in_autocompletemode);
    TEST(simple_latlon_search);
    TEST(simple_lonlat_search);
    TEST(simple_name_search);
    TEST(name_id_search);
    TEST(utf8_region_search);
    TEST(search_with_area);
    TEST(specific_countries);
    TEST(specific_features);
    TEST(specific_keywords);
    TEST(specific_language);
    TEST(specific_radius);
    TEST(specific_result_limit);
    TEST(specific_variants);

    /*
    // private function tests
    TEST(resolve_feature);
    TEST(resolve_name_variant);
    TEST(resolve_country);
    TEST(resolve_municipality);
    TEST(resolve_administrative);
    */
  }

};  // class tests

}  // namespace QueryTest

int main(void)
{
  cout << endl << "Query tester" << endl << "=============" << endl;
  Fmi::Database::PostgreSQLConnection::disableReconnect();
  QueryTest::tests t;
  return t.run();
}
