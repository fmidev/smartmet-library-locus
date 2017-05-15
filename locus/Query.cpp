// ======================================================================
/*!
 * \brief Implementation of class Locus::Query
 */
// ======================================================================

#include "Query.h"

#include <macgyver/StringConversion.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/make_shared.hpp>
#include <cmath>
#include <stdexcept>

const char* CLIENT_ENCODING = "UTF8";

using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Convert from UTF-8 to given locale
 */
// ----------------------------------------------------------------------

string from_utf(const string& name, const string& ansiname, const string& encoding)
{
  try
  {
    return boost::locale::conv::from_utf(name, encoding, boost::locale::conv::stop);
  }
  catch (...)
  {
    return ansiname;
  }
}

string from_utf(const string& name, const string& encoding)
{
  return boost::locale::conv::from_utf(name, encoding);
}

// ----------------------------------------------------------------------
/*!
 * \brief Utility algorithm to test if a container has a specific element
 */
// ----------------------------------------------------------------------

template <typename T, typename S>
bool contains(const T& theContainer, const S& theObject)
{
  typename T::const_iterator it = find(theContainer.begin(), theContainer.end(), theObject);
  return (it != theContainer.end());
}
}

namespace Locus
{
// ----------------------------------------------------------------------
/*!
 * \brief Default search radius
 */
// ----------------------------------------------------------------------

const float Query::default_radius = 50;

// ----------------------------------------------------------------------
/*!
 * \brief Alternate host constructor
 */
// ----------------------------------------------------------------------

Query::Query(const string& theHost,
             const string& theUser,
             const string& thePass,
             const string& theDatabase)
    : debug(false), recursive_query(false)
{
  /* Make a connection to the database */
  conn.open(theHost, theUser, thePass, theDatabase, CLIENT_ENCODING);
}

// ----------------------------------------------------------------------
/*!
 * \brief Alternate host constructor
 */
// ----------------------------------------------------------------------

Query::Query(const string& theHost,
             const string& theUser,
             const string& thePass,
             const string& theDatabase,
             const string& thePort)
    : debug(false), recursive_query(false)
{
  /* Make a connection to the database */
  conn.open(theHost, theUser, thePass, theDatabase, CLIENT_ENCODING, thePort);
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor closes the PosrgreSQL connection
 */
// ----------------------------------------------------------------------

Query::~Query() {}
// ----------------------------------------------------------------------
/*!
 * \brief Activate the correct language
 */
// ----------------------------------------------------------------------

void Query::SetOptions(const QueryOptions& theOptions)
{
  conn.setClientEncoding(theOptions.GetCharset());
}

// ----------------------------------------------------------------------
/*!
 * SetDebug-mode on or off. Debug-mode prints sql-queries and
 * error-messages
 *
 * \param value Boolean for mode: true=on, false=off
 * \return void
 */
// ----------------------------------------------------------------------

void Query::SetDebug(bool theFlag)
{
  debug = theFlag;
  conn.setDebug(theFlag);
}

// ----------------------------------------------------------------------
/*!
 * Resolves name for the feature code
 *
 * \param code Feature code
 * \return feature Name of the feature or false if not found
 */
// ----------------------------------------------------------------------

string Query::ResolveFeature(const QueryOptions& theOptions, const string& theCode)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eFeatureCode] = theCode;

  string sqlStmt = constructSQLStatement(eResolveFeature, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  string retval;

  if (!res.empty()) retval = res[0][0].as<string>();

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Helper method to return variant name for search result using
 * correct language
 *
 * \param id Database id for geoname
 * \return feature Name of the feature or false if not found
 */
// ----------------------------------------------------------------------

string Query::ResolveNameVariant(const QueryOptions& theOptions,
                                 const string& theId,
                                 const string& theSearchWord)
{
  // Uses name length to select shortest variant for language,
  // because there may be multiple variants like Tokio and
  // Tokion prefektuuri.

  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eGeonamesId] = theId;
  params[eSearchWord] = theSearchWord;

  string sqlStmt = constructSQLStatement(eResolveNameVariant, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  string retval;

  if (!res.empty()) retval = res[0][0].as<string>();

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Resolves name for country
 *
 * \param iso2 Iso country code
 * \return country Name of the country or false if not found
 */
// ----------------------------------------------------------------------

string Query::ResolveCountry(const QueryOptions& theOptions, const string& theIsoCode)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eCountryIso2Code] = theIsoCode;

  string sqlStmt = constructSQLStatement(eResolveCountry1, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  string retval;

  if (!res.empty())
  {
    retval = res[0][0].as<string>();
  }
  else
  {
    // If variant is not found use name in countries-table
    string sqlStmt = constructSQLStatement(eResolveCountry2, params);
    pqxx::result res = conn.executeNonTransaction(sqlStmt);

    if (!res.empty()) retval = res[0][0].as<string>();
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Helper method that resolves name for municipality id
 * \param id Municipality id
 * \return municipality Name of the municipality or "" if not found
 */
// ----------------------------------------------------------------------

string Query::ResolveMunicipality(const QueryOptions& theOptions, const string& theId)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eMunicipalityId] = theId;

  string sqlStmt = constructSQLStatement(eResolveMunicipality1, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  string retval;

  if (!res.empty())
  {
    retval = res[0][0].as<string>();
  }

  if (theOptions.GetLanguage() == "fi") return retval;

  sqlStmt = constructSQLStatement(eResolveMunicipality2, params);
  res = conn.executeNonTransaction(sqlStmt);

  // default is the original name
  if (!res.empty()) retval = res[0][0].as<string>();

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Resolves name for administrative area (state)
 *
 * \param code Administrative area code
 * \param country Country iso code
 * \return administrative Name of the state or false if not found
 */
// ----------------------------------------------------------------------

string Query::ResolveAdministrative(const string& theCode, const string& theCountry)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = QueryOptions();
  string adminCode = theCountry + "." + theCode;
  params[eAdminCode] = adminCode;

  string sqlStmt = constructSQLStatement(eResolveAdministrative, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  string retval;

  if (!res.empty())
  {
    retval = res[0][0].as<string>();
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Helper method that generates country conditions for sql-query
 *
 * \param query
 */
// ----------------------------------------------------------------------

void Query::AddCountryConditions(const QueryOptions& theOptions, string& theQuery) const
{
  // Nothing added if % or all is in the list
  const list<string>& countries = theOptions.GetCountries();
  if (!countries.empty() && !contains(countries, "%") && !contains(countries, "all"))
  {
    // Append to the query

    int n = 1;
    for (list<string>::const_iterator it = countries.begin(); it != countries.end(); ++it, ++n)
    {
      string country_iso2 = *it;
      Fmi::ascii_toupper(country_iso2);

      if (n == 1)
        theQuery += " AND geonames.countries_iso2 IN (";
      else
        theQuery += ",";
      theQuery += conn.quote(country_iso2);
    }

    theQuery += ")";
  }

  const list<string>& excluded_countries = theOptions.GetExcludedCountries();
  if (!excluded_countries.empty() && !contains(countries, "%") && !contains(countries, "all"))
  {
    // Append to the query

    int n = 1;
    for (list<string>::const_iterator it = excluded_countries.begin();
         it != excluded_countries.end();
         ++it, ++n)
    {
      string country_iso2 = *it;
      Fmi::ascii_tolower(country_iso2);

      if (n == 1)
        theQuery += " AND geonames.countries_iso2 NOT IN (";
      else
        theQuery += ",";
      theQuery += conn.quote(country_iso2);
    }

    theQuery += ")";
  }
}

// ----------------------------------------------------------------------
/*!
 * Helper method that generates feature conditions for sql-query
 *
 * \param query
 */
// ----------------------------------------------------------------------
void Query::AddFeatureConditions(const QueryOptions& theOptions, string& theQuery) const
{
  // Nothing added if % or all is in the list

  const list<string>& features = theOptions.GetFeatures();

  if (features.empty() || contains(features, "%") || contains(features, "all")) return;

  // Append to the query

  int n = 1;
  for (list<string>::const_iterator it = features.begin(); it != features.end(); ++it, ++n)
  {
    theQuery += (n == 1 ? " AND (" : " OR ");
    theQuery += "features_code=";
    theQuery += conn.quote(*it);
  }

  theQuery += ")";
}

// ----------------------------------------------------------------------
/*!
* Helper method that generates keyword conditions for sql-query
*
* \param query sql-string
* \return query sql-string with keyword conditions
*/
// ----------------------------------------------------------------------

void Query::AddKeywordConditions(const QueryOptions& theOptions, string& theQuery) const
{
  const list<string>& keywords = theOptions.GetKeywords();

  if (keywords.empty() || contains(keywords, "%") || contains(keywords, "all")) return;

  // Append to the query

  int n = 1;
  for (list<string>::const_iterator it = keywords.begin(); it != keywords.end(); ++it)
  {
    if (n == 1)
    {
      theQuery +=
          " AND geonames.id IN (SELECT geonames_id FROM keywords_has_geonames WHERE keyword IN (";
      theQuery += conn.quote(*it);
    }
    else
    {
      theQuery += ",";
      theQuery += conn.quote(*it);
    }
    n++;
  }

  if (n > 1) theQuery += "))";
}

// ----------------------------------------------------------------------
/*!
 * Public method for fetching locations by name
 *
 * \param name Place name
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByName(const QueryOptions& theOptions, const string& theName)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eLocationName] = theName;

  SetOptions(theOptions);

  // This allows queries like Helsinki, Finland

  vector<string> qparts;
  if (!theName.empty()) boost::algorithm::split(qparts, theName, boost::algorithm::is_any_of(","));
  string searchword = (qparts.empty() ? string("") : qparts[0]);

  params[eSearchWord] = searchword;

  string collation = theOptions.GetCollation();

  // Set country priorities

  const list<string>& countries = theOptions.GetCountries();

  std::string country_priorities;
  if (countries.size() > 1)
  {
    country_priorities += ", CASE countries_iso2 WHEN ";
    int n = 1;
    for (list<string>::const_iterator it = countries.begin(); it != countries.end(); ++it, ++n)
    {
      if (n == 1)
      {
        country_priorities += conn.quote(*it);
        country_priorities += " THEN 1 ";
      }
      else
      {
        country_priorities += " WHEN ";
        country_priorities += conn.quote(*it);
        country_priorities += " THEN ";
        country_priorities += Fmi::to_string(n);
      }
    }
    if (n > 1) country_priorities += " ELSE 1000 END as country_priority ";
  }
  params[eCountryPriorities] = country_priorities;

  // Set feature priorities

  const list<string>& features = theOptions.GetFeatures();

  std::string feature_priorities;
  if (features.size() > 1)
  {
    feature_priorities += ", CASE features_code WHEN ";
    int n = 1;
    for (list<string>::const_iterator it = features.begin(); it != features.end(); ++it, ++n)
    {
      if (n == 1)
      {
        feature_priorities += conn.quote(*it);
        feature_priorities += " THEN 1 ";
      }
      else
      {
        feature_priorities += " WHEN ";
        feature_priorities += conn.quote(*it);
        feature_priorities += " THEN ";
        feature_priorities += Fmi::to_string(n);
      }
    }
    if (n > 1) feature_priorities += " ELSE 1000 END as feature_priority ";
  }
  params[eFeaturePriorities] = feature_priorities;

  string sqlStmt = constructSQLStatement(eFetchByName, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  // Create result list
  return_type locations;
  if (qparts.size() == 2)
    locations = build_locations(theOptions, res, searchword, qparts[1]);
  else
    locations = build_locations(theOptions, res, searchword);

  if (locations.size() > 0) return locations;

  // Prevent endless recursion

  if (recursive_query) return locations;

  if (theOptions.GetFullCountrySearch())
  {
    // Search all countries

    if (debug) cout << "Do full country seach because limited search didn't return results" << endl;

    QueryOptions newoptions = theOptions;
    newoptions.SetCountries("%");
    recursive_query = true;
    locations = FetchByName(newoptions, theName);
    recursive_query = false;
  }

  return locations;
}

// ----------------------------------------------------------------------
/*!
 * Alias for FetchByLonLat-method because some people don't like lon,lat order
 *
 * \param lat Latitude
 * \param lon Longitude
 * \param radius Maximum distance from point in kilometers.
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByLatLon(const QueryOptions& theOptions,
                                        float theLatitude,
                                        float theLongitude,
                                        float theRadius)
{
  return FetchByLonLat(theOptions, theLongitude, theLatitude, theRadius);
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching locations close to some lon,lat point
 *
 * \param lat Latitude
 * \param lon Longitude
 * \param radius Maximum distance from point in kilometers.
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByLonLat(const QueryOptions& theOptions,
                                        float theLongitude,
                                        float theLatitude,
                                        float theRadius)
{
  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eLongitude] = theLongitude;
  params[eLatitude] = theLatitude;
  params[eRadius] = theRadius;

  SetOptions(theOptions);

  string sqlStmt = constructSQLStatement(eFetchByLonLat, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  auto ret = build_locations(theOptions, res, "", "");

  auto copyret = boost::make_shared<Locus::Query::return_type>(ret);

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching location by unique id
 *
 * \param id unique fminames id for location
 * \return locations Array of SimpleLocation objects (0/1)
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchById(const QueryOptions& theOptions, int theId)
{
  SetOptions(theOptions);

  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eGeonameId] = theId;

  string sqlStmt = constructSQLStatement(eFetchById, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  if (res.size() == 0 && theId >= 10000000) return FetchById(theOptions, -theId);

  return build_locations(theOptions, res, "", "");
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching locations by keyword
 *
 * \param keyword keyword
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByKeyword(const QueryOptions& theOptions, const string& theKeyword)
{
  SetOptions(theOptions);

  // We always want all the names in the keyword, not just the default 100
  QueryOptions options = theOptions;
  options.SetResultLimit(0);

  map<SQLQueryParameterId, boost::any> params;
  params[eKeyword] = theKeyword;
  params[eQueryOptions] = options;

  auto locations = boost::make_shared<Locus::Query::return_type>();

  string sqlStmt = constructSQLStatement(eFetchByKeyword1, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  if (res.size() != 1) return *locations;

  sqlStmt = constructSQLStatement(eFetchByKeyword2, params);

  res = conn.executeNonTransaction(sqlStmt);

  auto ret = build_locations(options, res, "", "");

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching number of locations by keyword
 *
 * \param keyword keyword
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

unsigned int Query::CountKeywordLocations(const QueryOptions& theOptions, const string& theKeyword)
{
  SetOptions(theOptions);

  map<SQLQueryParameterId, boost::any> params;
  params[eQueryOptions] = theOptions;
  params[eKeyword] = theKeyword;

  auto locations = boost::make_shared<Locus::Query::return_type>();

  string sqlStmt = constructSQLStatement(eCountKeywordLocations, params);
  pqxx::result res = conn.executeNonTransaction(sqlStmt);

  unsigned int ret = res[0]["count"].as<unsigned int>();

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Build a list of locations from query result
 *
 * Note: This implementation differs from the PHP version in that
 *       we implement a cache for storing query results which are
 *       likely to be repeated for example in keyword searches.
 */
// ----------------------------------------------------------------------

Query::return_type Query::build_locations(const QueryOptions& theOptions,
                                          const pqxx::result& theR,
                                          const string& theSearchWord,
                                          const string& theArea /* = ""*/)
{
  return_type locations;

  if (theR.empty()) return locations;

  // Does the result have a field for overriding names?
  bool has_override_field = false;
  for (unsigned int field = 0; !has_override_field && field < theR.columns(); ++field)
  {
    if (string(theR.column_name(field)) == string("override_name")) has_override_field = true;
  }

  // Caches subquery results

  map<string, string> country_cache;       // iso2 --> country
  map<string, string> feature_cache;       // feature_code --> description
  map<string, string> municipality_cache;  // id --> municipality
  map<string, string> admin_cache;         // name|iso2 --> admin

  // Process one location at a time

  for (pqxx::result::const_iterator row = theR.begin(); row != theR.end(); ++row)
  {
    // Do not handle locations without timezones. This is just a safety check,
    // NULL timezones should be removed already in the SQL query, otherwise
    // you might get zero results if the result count limit is 1.

    if (row["timezone"].is_null()) continue;

    // Determine name
    string name = (!row["name"].is_null() ? row["name"].as<string>() : "NULL");

    // Use override if there is one

    bool override_done = false;
    if (has_override_field)
    {
      string altname =
          (!row["override_name"].is_null() ? row["override_name"].as<string>() : "NULL");
      if (!altname.empty() && altname != "NULL")
      {
        name = altname;
        override_done = true;
      }
    }

    // Search for possible translations

    if (!override_done && !theOptions.GetLanguage().empty())
    {
      string variant;
      string id = row["id"].as<string>();
      if (!theOptions.GetAutoCompleteMode())
        variant = ResolveNameVariant(theOptions, id);
      else
        variant = ResolveNameVariant(theOptions, id, theSearchWord);

      if (!variant.empty()) name = variant;
    }

    if ((!row["ansiname"].is_null()) && (theOptions.GetCharset() != "utf-8"))
      name = from_utf(name, row["ansiname"].as<string>(), theOptions.GetCharset());

    // Elevation

    int elevation = 0;

    if (!row["elevation"].is_null() && (row["elevation"].as<int>() != 0))
      elevation = row["elevation"].as<int>();
    else if (!row["dem"].is_null())
      elevation = row["dem"].as<int>();

    // Country and description

    string country;
    string iso2 = "";
    if (!row["iso2"].is_null())
    {
      iso2 = row["iso2"].as<string>();
      const auto pos = country_cache.find(iso2);
      if (pos != country_cache.end())
        country = pos->second;
      else
      {
        country = ResolveCountry(theOptions, iso2);
        if (theOptions.GetCharset() != "utf-8")
          country = from_utf(country, theOptions.GetCharset());
        country_cache[iso2] = country;
      }
    }

    string description;
    string features_code = "";
    if (!row["features_code"].is_null())
    {
      features_code = row["features_code"].as<string>();
      const auto pos = feature_cache.find(features_code);
      if (pos != feature_cache.end())
        description = pos->second;
      else
      {
        description = ResolveFeature(theOptions, features_code);
        feature_cache[features_code] = description;
      }
    }

    // Administrative areas

    string administrative = "";
    if (!row["municipalities_id"].is_null())
    {
      string municipalities_id = row["municipalities_id"].as<string>();

      if (!municipalities_id.empty())
      {
        const auto pos = municipality_cache.find(municipalities_id);
        if (pos != municipality_cache.end())
          administrative = pos->second;
        else
        {
          administrative = ResolveMunicipality(theOptions, municipalities_id);
          if (theOptions.GetCharset() != "utf-8")
            administrative = from_utf(administrative, theOptions.GetCharset());
          municipality_cache[municipalities_id] = administrative;
        }
      }
      else
      {
        string admin1 = row["admin1"].as<string>();
        if (!admin1.empty())
        {
          string iso2 = row["iso2"].as<string>();
          string key = admin1 + '|' + iso2;
          const auto pos = admin_cache.find(key);
          if (pos != admin_cache.end())
            administrative = pos->second;
          else
          {
            administrative = ResolveAdministrative(admin1, iso2);
            if (theOptions.GetCharset() != "utf-8")
              administrative = from_utf(administrative, theOptions.GetCharset());
            admin_cache[key] = administrative;
          }
        }
      }
    }

    // Check if area is correct
    bool ok = true;

    if (!theArea.empty())
    {
      string lc_area = boost::algorithm::to_lower_copy(theArea);
      if (lc_area == boost::algorithm::to_lower_copy(country))
        ok = true;
      else if (lc_area == boost::algorithm::to_lower_copy(administrative))
        ok = true;
      else
        ok = false;
    }

    if (ok)
    {
      locations.push_back(SimpleLocation(name,
                                         row["lon"].as<float>(),
                                         row["lat"].as<float>(),
                                         country,
                                         features_code,
                                         description,
                                         row["timezone"].as<string>(),
                                         administrative,
                                         row["population"].as<int>(),
                                         iso2,
                                         row["id"].as<int>(),
                                         elevation));
    }

    // See if locations-sequence is already long enough

    if (theOptions.GetResultLimit() > 0 && locations.size() >= theOptions.GetResultLimit())
    {
      break;
    }
  }

  // Sort exact matchs first if autocompletemode

  if (theOptions.GetAutoCompleteMode())
  {
    vector<SimpleLocation> bestmatches;
    vector<SimpleLocation> secondarymatches;
    unsigned int n;

    // Remove "%" from searchword
    string tmp = theSearchWord.substr(0, theSearchWord.size() - 1);

    for (n = 0; n < locations.size(); n++)
    {
      if (boost::iequals(tmp, locations[n].name))
        bestmatches.push_back(locations[n]);
      else
        secondarymatches.push_back(locations[n]);
    }

    locations.clear();

    for (n = 0; n < bestmatches.size(); n++)
    {
      locations.push_back(bestmatches[n]);
    }

    for (n = 0; n < secondarymatches.size(); n++)
    {
      locations.push_back(secondarymatches[n]);
    }
  }

  return locations;
}

string Query::constructSQLStatement(SQLQueryId theQueryId,
                                    const map<SQLQueryParameterId, boost::any>& theParams)
{
  std::string sql;

  const QueryOptions& theOptions =
      boost::any_cast<const QueryOptions&>(theParams.at(eQueryOptions));

  switch (theQueryId)
  {
    case eResolveFeature:
    {
      string theCode = boost::any_cast<string>(theParams.at(eFeatureCode));
      sql += "SELECT shortdesc FROM features WHERE code=";
      sql += conn.quote(theCode);
    }
    break;
    case eResolveNameVariant:
    {
      string theGeonamesId = boost::any_cast<string>(theParams.at(eGeonamesId));
      string theSearchWord = boost::any_cast<string>(theParams.at(eSearchWord));
      string language = theOptions.GetLanguage();
      Fmi::ascii_tolower(language);

      if (!theOptions.GetAutoCompleteMode())
      {
        sql += "SELECT name,length(name) AS l, priority FROM alternate_geonames WHERE geonames_id=";
        sql += conn.quote(theGeonamesId);
        sql += " AND language=";
        sql += conn.quote(language);
        sql += " AND historic=false ORDER BY priority ASC, preferred DESC, l ASC, name ASC LIMIT 1";
      }
      else
      {
        sql += "SELECT name,length(name) As l, priority FROM alternate_geonames WHERE geonames_id=";
        sql += conn.quote(theGeonamesId);
        sql += " AND language=";
        sql += conn.quote(language);
        sql += " AND name LIKE ";
        sql += conn.quote(theSearchWord);
        sql += " AND historic=false ORDER BY priority ASC, preferred DESC, l ASC, name ASC LIMIT 1";
      }
      break;
    }
    case eResolveCountry1:
    case eResolveCountry2:
    {
      string iso2 = boost::any_cast<string>(theParams.at(eCountryIso2Code));
      string language = theOptions.GetLanguage();
      Fmi::ascii_tolower(language);

      if (theQueryId == eResolveCountry1)
      {
        sql +=
            "SELECT alternate_geonames.name AS name, length(alternate_geonames.name) AS l"
            " FROM geonames, alternate_geonames"
            " WHERE geonames.features_code='PCLI'"
            " AND geonames.countries_iso2=";
        sql += conn.quote(iso2);
        sql +=
            " AND geonames.id=alternate_geonames.geonames_id"
            " AND alternate_geonames.language=";
        sql += conn.quote(language);
        sql += " ORDER BY preferred DESC, alternate_geonames.priority ASC, l ASC LIMIT 1";
      }
      else
      {
        sql += "SELECT name FROM countries WHERE iso2=";
        sql += conn.quote(iso2);
      }
      break;
    }
    case eResolveMunicipality1:
    case eResolveMunicipality2:
    {
      string theMunicipalityId = boost::any_cast<string>(theParams.at(eMunicipalityId));
      string language = theOptions.GetLanguage();
      Fmi::ascii_tolower(language);

      if (theQueryId == eResolveMunicipality1)
      {
        sql += "SELECT name FROM municipalities WHERE id=";
        sql += conn.quote(theMunicipalityId);
      }
      else
      {
        sql += "SELECT name FROM alternate_municipalities WHERE municipalities_id=";
        sql += conn.quote(theMunicipalityId);
        sql += " AND language=";
        sql += conn.quote(language);
      }
      break;
    }
    case eResolveAdministrative:
    {
      string theAdminCode = boost::any_cast<string>(theParams.at(eAdminCode));
      Fmi::ascii_tolower(theAdminCode);
      sql += "SELECT name FROM admin1codes WHERE code=";
      sql += conn.quote(theAdminCode);
      break;
    }
    case eFetchByName:
    {
      if (theOptions.GetSearchVariants()) sql += "(";

      string theSearchWord = boost::any_cast<string>(theParams.at(eSearchWord));
      string theCountryPriorities = boost::any_cast<string>(theParams.at(eCountryPriorities));
      string theFeaturePriorities = boost::any_cast<string>(theParams.at(eFeaturePriorities));
      // boost::algorithm::to_lower(theSearchWord);

      sql +=
          "SELECT DISTINCT geonames.name AS name,"
          " geonames.ansiname AS ansiname,"
          " lat, lon, countries_iso2 AS iso2,"
          " features_code, timezone, geonames.id as id, geonames.priority as geonames_priority,"
          " municipalities_id, admin1, population, "
          " elevation, dem, "
          " CASE WHEN population>50000 THEN population ELSE 0 END AS population_priority ";
      sql += theCountryPriorities;
      sql += ' ';
      sql += theFeaturePriorities;
      sql += " FROM geonames WHERE LOWER(geonames.name) LIKE LOWER(";
      sql += conn.quote(theSearchWord);
      sql += ')';

      if (conn.collateSupported())
      {
        sql += " COLLATE ";
        sql += conn.quote(theOptions.GetCollation());
      }

      // PHP version does not do this, but we cannot tolerate it in brainstorm
      sql += " AND timezone IS NOT NULL";

      if (theOptions.GetPopulationMin() > 0)
      {
        sql += " AND population>=";
        sql += Fmi::to_string(theOptions.GetPopulationMin());
      }
      if (theOptions.GetPopulationMax() > 0)
      {
        sql += " AND population<=";
        sql += Fmi::to_string(theOptions.GetPopulationMax());
      }

      AddFeatureConditions(theOptions, sql);
      AddCountryConditions(theOptions, sql);
      AddKeywordConditions(theOptions, sql);

      if (theOptions.GetSearchVariants())
      {
        string language = theOptions.GetLanguage();
        Fmi::ascii_tolower(language);

        sql +=
            ") UNION (SELECT DISTINCT geonames.name AS name,"
            " geonames.ansiname AS ansiname, lat, lon,"
            " countries_iso2 AS iso2, features_code, timezone,"
            " geonames.id as id, geonames.priority as geonames_priority, municipalities_id,"
            " admin1, population, "
            " elevation, dem, "
            " CASE WHEN population>50000 THEN population ELSE 0 END AS population_priority ";
        sql += theCountryPriorities;
        sql += ' ';
        sql += theFeaturePriorities;
        sql +=
            " FROM geonames, alternate_geonames WHERE LOWER(alternate_geonames.name) LIKE LOWER(";
        sql += conn.quote(theSearchWord);
        sql += ")";

        if (conn.collateSupported())
        {
          sql += " COLLATE ";
          sql += conn.quote(theOptions.GetCollation());
        }

        sql +=
            " AND alternate_geonames.geonames_id=geonames.id AND alternate_geonames.language LIKE ";
        sql += conn.quote(language);

        if (theOptions.GetPopulationMin() > 0)
        {
          sql += " AND population>=";
          sql += Fmi::to_string(theOptions.GetPopulationMin());
        }
        if (theOptions.GetPopulationMax() > 0)
        {
          sql += " AND population<=";
          sql += Fmi::to_string(theOptions.GetPopulationMax());
        }
        if (theOptions.GetAutoCompleteMode())
        {
          sql += " AND alternate_geonames.language=";
          sql += conn.quote(language);
        }

        AddFeatureConditions(theOptions, sql);
        AddCountryConditions(theOptions, sql);
        AddKeywordConditions(theOptions, sql);
        sql += ')';
      }

      sql += " ORDER BY geonames_priority, population_priority DESC, ";
      sql += (theCountryPriorities.empty() ? "" : "country_priority, ");
      sql += (theFeaturePriorities.empty() ? "" : "feature_priority, ");
      sql += " population DESC, name";

      if (conn.collateSupported())
      {
        sql += " COLLATE ";
        sql += conn.quote(theOptions.GetCollation());
      }

      break;
    }
    case eFetchByLonLat:
    {
      float theLongitude = boost::any_cast<float>(theParams.at(eLongitude));
      float theLatitude = boost::any_cast<float>(theParams.at(eLatitude));
      float theRadius = boost::any_cast<float>(theParams.at(eRadius));

      sql +=
          "WITH candidates AS ("
          "SELECT geonames.id AS id, geonames.name AS name,"
          "geonames.ansiname AS ansiname, lat, lon,"
          "countries_iso2 AS iso2, features_code, timezone,"
          "population, elevation, dem, municipalities_id,"
          "admin1, ST_Distance(ST_GeographyFromText('POINT(";
      sql += Fmi::to_string(theLongitude);
      sql += ' ';
      sql += Fmi::to_string(theLatitude);
      sql += ")'), the_geog) as distance FROM geonames WHERE ";

      // PHP version does not do this, but we cannot tolerate it in brainstorm
      sql += " timezone IS NOT NULL";

      if (theOptions.GetPopulationMin() > 0)
      {
        sql += " AND population>=";
        sql += Fmi::to_string(theOptions.GetPopulationMin());
      }
      if (theOptions.GetPopulationMax() > 0)
      {
        sql += " AND population<=";
        sql += Fmi::to_string(theOptions.GetPopulationMax());
      }

      AddCountryConditions(theOptions, sql);
      AddFeatureConditions(theOptions, sql);
      AddKeywordConditions(theOptions, sql);

      sql += " ORDER BY the_geom <-> ST_GeomFromText('POINT(";
      sql += Fmi::to_string(theLongitude);
      sql += ' ';
      sql += Fmi::to_string(theLatitude);
      sql += ")',4326)";

      // <-> ordering is appriximate, so we need to fetch more results
      // than wanted to make sure the final list is properly ordered.
      // HOWEVER: It is important to keep this number small, increasing
      // the margin increases execution time rapidly

      const int limit_safety_margin = 10;

      if (theOptions.GetResultLimit() > 0)
      {
        sql += " LIMIT ";
        sql += Fmi::to_string(limit_safety_margin + theOptions.GetResultLimit());
      }

      sql += ") SELECT * from candidates";

      // This best best done in the outer select since postgresql 9.1
      if (theRadius > 0)
      {
        sql += " WHERE distance<=";
        sql += Fmi::to_string(theRadius * 1000);
      }

      sql += " ORDER BY distance";
      if (theOptions.GetResultLimit() > 0)
      {
        sql += " LIMIT ";
        sql += Fmi::to_string(theOptions.GetResultLimit());
      }

      break;
    }
    case eFetchById:
    {
      int theId = boost::any_cast<int>(theParams.at(eGeonameId));

      sql +=
          "SELECT id, name, ansiname, lat, lon, countries_iso2 AS iso2, features_code,"
          " timezone, municipalities_id, admin1, population,"
          " elevation, dem "
          " FROM geonames WHERE id=";
      sql += Fmi::to_string(theId);
      break;
    }
    case eFetchByKeyword1:
    case eFetchByKeyword2:
    case eFetchByKeyword3:
    {
      string theKeyword = boost::any_cast<string>(theParams.at(eKeyword));

      if (theQueryId == eFetchByKeyword1)
      {
        sql += "SELECT keyword FROM keywords WHERE keyword=";
        sql += conn.quote(theKeyword);
      }
      else if (theQueryId == eFetchByKeyword2)
      {
        sql +=
            "SELECT geonames.id AS id, geonames.name AS name,\n "
            "geonames.ansiname AS ansiname,lat,lon,\n "
            "countries_iso2 AS iso2, features_code, timezone,\n "
            "population, elevation, dem,\n "
            "municipalities_id, admin1,\n "
            "keywords_has_geonames.name AS override_name FROM \n "
            "geonames, keywords_has_geonames WHERE\n "
            "keywords_has_geonames.keyword=";
        sql += conn.quote(theKeyword);
        sql +=
            " AND geonames.id=keywords_has_geonames.geonames_id"
            " ORDER BY name";
      }
      else if (theQueryId == eFetchByKeyword3)
      {
        // long version
        sql +=
            "SELECT georesults.*,\n"
            "       municipalities.name AS mname,\n"
            "       altname_translations.name AS altname,\n"
            "       alternate_municipalities.name AS altmname,\n"
            "       admin1codes.name AS adminname,\n"
            "       iso2_translations.name AS altcname\n"
            "\n"
            "FROM\n"
            "(\n"

            // -- basic geonames results

            "  SELECT geonames.admin1,\n"
            "         geonames.ansiname AS ansiname,\n"
            "         geonames.countries_iso2 AS iso2,\n"
            "         geonames.elevation,\n"
            "         geonames.features_code,\n"
            "         geonames.dem,\n"
            "         geonames.id AS id,\n"
            "         geonames.lat,\n"
            "         geonames.lon,\n"
            "         geonames.municipalities_id,\n"
            "         geonames.name AS name,\n"
            "         geonames.population,\n"
            "         geonames.timezone,\n"
            "         countries.name AS cname,\n"
            "         features.shortdesc AS shortdesc,\n"
            "         keywords_has_geonames.name AS override_name\n"
            "  FROM geonames, keywords_has_geonames, features, countries\n"
            "  WHERE geonames.id=keywords_has_geonames.geonames_id\n"
            "  AND keywords_has_geonames.keyword=";
        sql += conn.quote(theKeyword);
        sql +=
            "  AND features.code=geonames.features_code\n"
            "  AND geonames.countries_iso2=countries.iso2\n"
            ")\n"
            "AS georesults\n"

            // -- left join to add municipality if available

            "LEFT JOIN municipalities\n"
            "ON (municipalities.id=georesults.municipalities_id\n"
            "    AND municipalities.countries_iso2=georesults.iso2)\n"
            "\n"
            "-- left join to add alternate municipality if available\n"
            "\n"
            "LEFT JOIN alternate_municipalities\n"
            "ON (georesults.municipalities_id=alternate_municipalities.municipalities_id\n"
            "    AND alternate_municipalities.language=";
        sql += conn.quote(theOptions.GetLanguage());
        sql +=
            "   )\n"
            "\n"
            "-- left join to add admin name if available\n"
            "\n"
            "LEFT JOIN admin1codes\n"
            "ON (admin1codes.code=georesults.admin1 AND admin1codes.geonames_id=georesults.id)\n"

            //  left join to add alternate name if available

            "LEFT JOIN\n"
            "(\n"
            "  SELECT id,name FROM\n"
            "  (\n"
            "    SELECT geonames.id AS id,\n"
            "           alternate_geonames.name AS name,\n"
            "           length(alternate_geonames.name) AS l\n"
            "    FROM geonames, alternate_geonames, keywords_has_geonames\n"
            "    WHERE geonames.id=alternate_geonames.geonames_id\n"
            "    AND keywords_has_geonames.geonames_id=geonames.id\n"
            "    AND keywords_has_geonames.keyword=";
        sql += conn.quote(theKeyword);
        sql += "    AND alternate_geonames.language=";
        sql += conn.quote(theOptions.GetLanguage());
        sql +=
            "    ORDER BY preferred DESC,l\n"
            "  )\n"
            "  AS altname_tmp\n"
            "  GROUP BY id, name \n"
            ")\n"
            "AS altname_translations\n"
            "ON (georesults.id=altname_translations.id)\n"

            // -- left join to add alternate country name if available

            "LEFT JOIN\n"
            "(\n"
            "  SELECT iso2,name FROM\n"
            "  (\n"
            "    SELECT countries_iso2 AS iso2,\n"
            "           alternate_geonames.name AS name,\n"
            "           length(alternate_geonames.name) AS l\n"
            "    FROM geonames, alternate_geonames\n"
            "    WHERE geonames.features_code='PCLI'\n"
            "    AND geonames.id=alternate_geonames.geonames_id\n"
            "    AND alternate_geonames.language=";
        sql += conn.quote(theOptions.GetLanguage());
        sql +=
            "    ORDER BY preferred DESC,l\n"
            "  )\n"
            "  AS iso2_tmp\n"
            "  GROUP BY iso2, name\n"
            ")\n"
            "AS iso2_translations\n"
            "ON (georesults.iso2=iso2_translations.iso2)\n"
            "ORDER BY id;\n";
      }
      break;
    }
    case eCountKeywordLocations:
    {
      string theKeyword = boost::any_cast<string>(theParams.at(eKeyword));

      sql += "SELECT count(*) AS count FROM keywords_has_geonames WHERE keyword=";
      sql += conn.quote(theKeyword);
      break;
    }
    default:
      break;
  }

  return sql;
}

}  // namespace Locus

// ======================================================================
