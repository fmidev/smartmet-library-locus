// ======================================================================
/*!
 * \brief Implementation of class Locus::Query
 */
// ======================================================================

#include "Query.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/make_shared.hpp>
#include <macgyver/Exception.h>
#include <macgyver/Join.h>
#include <macgyver/StringConversion.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <fmt/format.h>


using namespace std;

namespace
{
const char* CLIENT_ENCODING = "UTF8";

// ----------------------------------------------------------------------
/*!
 * \brief Convert from UTF-8 to given locale
 */
// ----------------------------------------------------------------------

string from_utf(const string& name, const string& ansiname, const string& encoding)
{
  try
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
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

string from_utf(const string& name, const string& encoding)
{
  try
  {
    return boost::locale::conv::from_utf(name, encoding);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Utility algorithm to test if a container has a specific element
 */
// ----------------------------------------------------------------------

template <typename T, typename S>
bool contains(const T& theContainer, const S& theObject)
{
  try
  {
    const auto it = find(theContainer.begin(), theContainer.end(), theObject);
    return (it != theContainer.end());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::optional<int> find_column(
    const pqxx::result& theResult, const std::string& theColumnName)
{
  try
  {
    for (int i = 0; i < theResult.columns(); ++i)
    {
      if (theResult.column_name(i) == theColumnName)
        return i;
    }
    return std::nullopt;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

template <typename ValueType>
std::set<ValueType> get_unique_values(
    const pqxx::result& theResult, const std::string& theColumnName)
{
  try
  {
    std::set<ValueType> values;
    auto column = find_column(theResult, theColumnName);
    if (column)
    {
      for (const auto& row : theResult)
      {
        if (row.size() <= *column)
          continue;  // Skip rows that do not have the column
        const auto& value = row[*column];
        if (value.is_null())
          continue;  // Skip null values
        values.insert(value.as<ValueType>());
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace Locus
{
// ----------------------------------------------------------------------
/*!
 * \brief Default locale
 */
// ----------------------------------------------------------------------

boost::locale::generator locale_generator;
std::locale default_locale = locale_generator("fi_FI.UTF-8");

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
    : conn(new Fmi::Database::PostgreSQLConnection)
{
  try
  {
    /* Make a connection to the database */
    Fmi::Database::PostgreSQLConnectionOptions opt;
    opt.host = theHost;
    opt.username = theUser;
    opt.password = thePass;
    opt.database = theDatabase;
    opt.encoding = CLIENT_ENCODING;
    conn->open(opt);
    // conn.open(theHost, theUser, thePass, theDatabase, CLIENT_ENCODING);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
    : conn(new Fmi::Database::PostgreSQLConnection)
{
  try
  {
    /* Make a connection to the database */
    Fmi::Database::PostgreSQLConnectionOptions opt;
    opt.host = theHost;
    opt.port = boost::lexical_cast<unsigned int>(thePort);
    opt.username = theUser;
    opt.password = thePass;
    opt.database = theDatabase;
    opt.encoding = CLIENT_ENCODING;
    conn->open(opt);
    // conn.open(theHost, theUser, thePass, theDatabase, CLIENT_ENCODING, thePort);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Activate the correct language
 */
// ----------------------------------------------------------------------

void Query::SetOptions(const QueryOptions& theOptions)
{
  try
  {
    conn->setClientEncoding(theOptions.GetCharset());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * SetDebug-mode on or off. Debug-mode prints sql-queries and
 * error-messages
 *
 * \param theFlag Boolean for mode: true=on, false=off
 */
// ----------------------------------------------------------------------

void Query::SetDebug(bool theFlag)
{
  try
  {
    debug = theFlag;
    conn->setDebug(theFlag);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Query::cancel()
{
  conn->cancel();
}

// ----------------------------------------------------------------------
/*!
 * Helper method to return variant name for search result using
 * correct language
 *
 * \param theId Database id for geoname
 * \return feature Name of the feature or false if not found
 */
// ----------------------------------------------------------------------
string Query::ResolveNameVariant(const QueryOptions& theOptions,
                                 int theId,
                                 const string& theSearchWord)
{
  try
  {
    // Uses name length to select shortest variant for language,
    // because there may be multiple variants like Tokio and
    // Tokion prefektuuri.

    map<SQLQueryParameterId, std::any> params;
    params[eQueryOptions] = theOptions;
    params[eGeonamesId] = theId;
    params[eSearchWord] = theSearchWord;

    string sqlStmt = constructSQLStatement(eResolveNameVariant, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    string retval;

    if (!res.empty())
      retval = res[0][0].as<string>();

    return retval;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::map<int, std::string>
Query::ResolveNameVariants(const QueryOptions& theOptions,
                           const vector<int>& theIds)
{
  map<SQLQueryParameterId, std::any> params;
  params[eQueryOptions] = theOptions;

  std::map<int, std::string> retval;
  std::vector<int>::const_iterator it = theIds.begin();
  while (it != theIds.end())
  {
    constexpr const size_t max_ids = 1000;  // Limit the number of ids to prevent too large queries
    std::vector<int> currIds;
    while (it != theIds.end() && currIds.size() < max_ids)
      currIds.push_back(*it++);
    params[eGeonamesId] = std::move(currIds);
    std::string sqlStmt = constructSQLStatement(eResolveNameVariants, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);
    for (const auto& row : res)
    {
      if (row.size() < 2)
        continue;

      const int id = row[0].as<int>();
      std::string name = row[1].as<string>();

      // If name is empty or already present in result map then skip it
      if (!name.empty() and not retval.count(id))
      {
        retval[id] = name;
      }
    }
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * Helper method that generates country conditions for sql-query
 *
 * \param theQuery the SQL query yo modify
 */
// ----------------------------------------------------------------------

void Query::AddCountryConditions(const QueryOptions& theOptions, string& theQuery) const
{
  try
  {
    // Nothing added if % or all is in the list
    const list<string>& countries = theOptions.GetCountries();
    if (!countries.empty() && !contains(countries, "%") && !contains(countries, "all"))
    {
      // Append to the query

      int n = 1;
      for (auto it = countries.begin(); it != countries.end(); ++it, ++n)
      {
        string country_iso2 = *it;
        Fmi::ascii_toupper(country_iso2);

        if (n == 1)
          theQuery += " AND geonames.countries_iso2 IN (";
        else
          theQuery += ",";
        theQuery += conn->quote(country_iso2);
      }

      theQuery += ")";
    }

    const list<string>& excluded_countries = theOptions.GetExcludedCountries();
    if (!excluded_countries.empty() && !contains(countries, "%") && !contains(countries, "all"))
    {
      // Append to the query

      int n = 1;
      for (auto it = excluded_countries.begin(); it != excluded_countries.end(); ++it, ++n)
      {
        string country_iso2 = *it;
        Fmi::ascii_tolower(country_iso2);

        if (n == 1)
          theQuery += " AND geonames.countries_iso2 NOT IN (";
        else
          theQuery += ",";
        theQuery += conn->quote(country_iso2);
      }

      theQuery += ")";
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Helper method that generates feature conditions for sql-query
 *
 */
// ----------------------------------------------------------------------
void Query::AddFeatureConditions(const QueryOptions& theOptions, string& theQuery) const
{
  try
  {
    // Nothing added if % or all is in the list

    const list<string>& features = theOptions.GetFeatures();

    if (features.empty() || contains(features, "%") || contains(features, "all"))
      return;

    // Append to the query

    int n = 1;
    for (auto it = features.begin(); it != features.end(); ++it, ++n)
    {
      theQuery += (n == 1 ? " AND (" : " OR ");
      theQuery += "features_code=";
      theQuery += conn->quote(*it);
    }

    theQuery += ")";
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Helper method that generates keyword conditions for sql-query
 *
 * \param theQuery sql-string
 */
// ----------------------------------------------------------------------

void Query::AddKeywordConditions(const QueryOptions& theOptions, string& theQuery) const
{
  try
  {
    const list<string>& keywords = theOptions.GetKeywords();

    if (keywords.empty() || contains(keywords, "%") || contains(keywords, "all"))
      return;

    // Append to the query

    int n = 1;
    for (const auto& keyword : keywords)
    {
      if (n == 1)
      {
        theQuery +=
            " AND geonames.id IN (SELECT geonames_id FROM keywords_has_geonames WHERE keyword IN (";
        theQuery += conn->quote(keyword);
      }
      else
      {
        theQuery += ",";
        theQuery += conn->quote(keyword);
      }
      n++;
    }

    if (n > 1)
      theQuery += "))";
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Public method for fetching locations by name
 *
 * \param theName Place name
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByName(const QueryOptions& theOptions, const string& theName)
{
  try
  {
    map<SQLQueryParameterId, std::any> params;
    QueryOptions opts = theOptions;
    if (!opts.GetNameType().empty())
      opts.SetLanguage(opts.GetNameType());
    params[eQueryOptions] = opts;
    params[eLocationName] = theName;

    SetOptions(theOptions);

    // This allows queries like Helsinki, Finland

    vector<string> qparts;
    if (!theName.empty())
      boost::algorithm::split(qparts, theName, boost::algorithm::is_any_of(","));
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
      for (auto it = countries.begin(); it != countries.end(); ++it, ++n)
      {
        if (n == 1)
        {
          country_priorities += conn->quote(*it);
          country_priorities += " THEN 1 ";
        }
        else
        {
          country_priorities += " WHEN ";
          country_priorities += conn->quote(*it);
          country_priorities += " THEN ";
          country_priorities += Fmi::to_string(n);
        }
      }
      if (n > 1)
        country_priorities += " ELSE 1000 END as country_priority ";
    }
    params[eCountryPriorities] = country_priorities;

    // Set feature priorities

    const list<string>& features = theOptions.GetFeatures();

    std::string feature_priorities;
    if (features.size() > 1)
    {
      feature_priorities += ", CASE features_code WHEN ";
      int n = 1;
      for (auto it = features.begin(); it != features.end(); ++it, ++n)
      {
        if (n == 1)
        {
          feature_priorities += conn->quote(*it);
          feature_priorities += " THEN 1 ";
        }
        else
        {
          feature_priorities += " WHEN ";
          feature_priorities += conn->quote(*it);
          feature_priorities += " THEN ";
          feature_priorities += Fmi::to_string(n);
        }
      }
      if (n > 1)
        feature_priorities += " ELSE 1000 END as feature_priority ";
    }
    params[eFeaturePriorities] = feature_priorities;

    string sqlStmt = constructSQLStatement(eFetchByName, params);

    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    // Create result list
    return_type locations;
    if (qparts.size() == 2)
      locations = build_locations(theOptions, res, searchword, qparts[1]);
    else
      locations = build_locations(theOptions, res, searchword);

    if (!locations.empty())
      return locations;

    // Prevent endless recursion

    if (recursive_query)
      return locations;

    if (theOptions.GetFullCountrySearch())
    {
      // Search all countries

      if (debug)
        cout << "Do full country seach because limited search didn't return results" << endl;

      QueryOptions newoptions = theOptions;
      newoptions.SetCountries("%");
      recursive_query = true;
      locations = FetchByName(newoptions, theName);
      recursive_query = false;
    }

    return locations;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Alias for FetchByLonLat-method because some people don't like lon,lat order
 *
 * \param theLatitude Latitude
 * \param theLongitude Longitude
 * \param theRadius Maximum distance from point in kilometers.
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByLatLon(const QueryOptions& theOptions,
                                        float theLatitude,
                                        float theLongitude,
                                        float theRadius)
{
  try
  {
    return FetchByLonLat(theOptions, theLongitude, theLatitude, theRadius);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching locations close to some lon,lat point
 *
 * \param theLatitude Latitude
 * \param theLongitude Longitude
 * \param theRadius Maximum distance from point in kilometers.
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByLonLat(const QueryOptions& theOptions,
                                        float theLongitude,
                                        float theLatitude,
                                        float theRadius)
{
  try
  {
    map<SQLQueryParameterId, std::any> params;
    params[eQueryOptions] = theOptions;
    params[eLongitude] = theLongitude;
    params[eLatitude] = theLatitude;
    params[eRadius] = theRadius;

    SetOptions(theOptions);

    string sqlStmt = constructSQLStatement(eFetchByLonLat, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    auto ret = build_locations(theOptions, res, "", "");

    auto copyret = std::make_shared<Locus::Query::return_type>(ret);

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching location by unique id
 *
 * \param theId unique fminames id for location
 * \return locations Array of SimpleLocation objects (0/1)
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchById(const QueryOptions& theOptions, int theId)
{
  try
  {
    SetOptions(theOptions);

    map<SQLQueryParameterId, std::any> params;
    params[eQueryOptions] = theOptions;
    params[eGeonameId] = theId;

    string sqlStmt = constructSQLStatement(eFetchById, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    if (res.empty() && theId >= 10000000)
      return FetchById(theOptions, -theId);

    return build_locations(theOptions, res, "", "");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching locations by keyword
 *
 * \param theKeyword keyword
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

Query::return_type Query::FetchByKeyword(const QueryOptions& theOptions, const string& theKeyword)
{
  try
  {
    SetOptions(theOptions);

    // We always want all the names in the keyword, not just the default 100
    QueryOptions options = theOptions;
    options.SetResultLimit(0);

    map<SQLQueryParameterId, std::any> params;
    params[eKeyword] = theKeyword;
    params[eQueryOptions] = options;

    auto locations = std::make_shared<Locus::Query::return_type>();

    string sqlStmt = constructSQLStatement(eFetchByKeyword1, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    if (res.size() != 1)
      return *locations;

    sqlStmt = constructSQLStatement(eFetchByKeyword2, params);

    res = conn->executeNonTransaction(sqlStmt);

    auto ret = build_locations(options, res, "", "");

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Method for fetching number of locations by keyword
 *
 * \param theKeyword keyword
 * \return locations Array of SimpleLocation objects
 */
// ----------------------------------------------------------------------

unsigned int Query::CountKeywordLocations(const QueryOptions& theOptions, const string& theKeyword)
{
  try
  {
    SetOptions(theOptions);

    map<SQLQueryParameterId, std::any> params;
    params[eQueryOptions] = theOptions;
    params[eKeyword] = theKeyword;

    auto locations = std::make_shared<Locus::Query::return_type>();

    string sqlStmt = constructSQLStatement(eCountKeywordLocations, params);
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    return res[0]["count"].as<unsigned int>();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::map<int, std::string>
Query::getNameVariants(
    const QueryOptions& theOptions,
    const pqxx::result& theR,
    const string& theSearchWord)
{
  std::map<int, std::string> name_variants;
  std::vector<int> variant_resolve_postponed;

  // Does the result have a field for overriding names?
  auto override_field_ind = find_column(theR, "override_name");
  bool has_override_field = bool(override_field_ind);
  for (unsigned int field = 0; !has_override_field && field < unsigned(theR.columns()); ++field)
  {
    if (string(theR.column_name(field)) == string("override_name"))
      has_override_field = true;
  }

  for (pqxx::result::const_iterator row = theR.begin(); row != theR.end(); ++row)
  {
    if (row["timezone"].is_null())
      continue;

    auto id = row["id"].as<int>();

    // Determine name
    string name;

    // Use override if there is one

    bool override_done = false;
    if (override_field_ind)
    {
      const auto & override = row[*override_field_ind];
      string altname = !override.is_null() ? override.as<string>() : "NULL";
      if (!altname.empty() && altname != "NULL")
      {
        name = altname;
        override_done = true;
      }
    }

    // Search for possible translations (postpone resolution if one can do it
    // for several sites in the same SQL request)

    if (!override_done && !theOptions.GetLanguage().empty())
    {
      string variant;
      if (!theOptions.GetAutoCompleteMode())
      {
        variant_resolve_postponed.push_back(id);
      }
      else
      {
        variant = ResolveNameVariant(theOptions, id, theSearchWord);
      }

      if (!variant.empty())
        name = variant;
    }

    if (!name.empty())
      name_variants[id] = name;  // Store current name for later use
  }

  // Resolve postponed name variants
  if (!variant_resolve_postponed.empty())
  {
    std::map<int, std::string> variants =
        ResolveNameVariants(theOptions, variant_resolve_postponed);

    for (const auto& item : variants)
    {
      const auto& id = item.first;
      const auto& name = item.second;

      // If name is empty or already present in result map then skip it
      if (!name.empty() && !name_variants.count(id))
        name_variants[id] = name;
    }
  }

  return name_variants;
}

std::vector<std::string> Query::getLanguageCodes(const std::string& language)
{
  std::vector<std::string> codes = get_iso639_table()->get_codes(language);
  if (codes.empty())
    codes.push_back(language);  // If no codes found, use the language itself
  return codes;
}


std::map<std::string, std::string> Query::getFeatures(
    const QueryOptions& theOptions,
    const pqxx::result& theR)
{
  std::map<std::string, std::string> features;
  std::set<std::string> feature_codes = get_unique_values<string>(theR, "features_code");
  if (feature_codes.empty())
    return features;  // No features to process

  constexpr const char* sql = "SELECT code, shortdesc FROM features WHERE code IN ({:s})";
  std::string sqlStmt = fmt::format(sql, quote(feature_codes));
  pqxx::result res = conn->executeNonTransaction(sqlStmt);
  for (const auto& row : res)
  {
    if (row.size() < 2)
      continue;  // Skip rows that do not have the expected columns
    std::string code = row[0].as<std::string>();
    std::string shortdesc = row[1].as<std::string>();
    if (!shortdesc.empty())
    {
        features[code] = shortdesc;
    }
  }
  return features;
}


std::map<std::string, std::string> Query::getCountryNames(
    const QueryOptions& theOptions,
    const pqxx::result& theR)
try
{
  constexpr const char* sql1 =
    "SELECT"
    " geonames.countries_iso2 AS iso2,"
    " alternate_geonames.name AS name,"
    " length(alternate_geonames.name) AS l "
    "FROM"
    " geonames,"
    " alternate_geonames "
    "WHERE"
    " geonames.features_code='PCLI'"
    " AND geonames.countries_iso2 IN ({:s})"
    " AND geonames.id=alternate_geonames.geonames_id"
    " AND alternate_geonames.language IN ({:s}) "
    "ORDER BY"
    " preferred DESC,"
    " alternate_geonames.priority ASC,"
    " l ASC";

  constexpr const char* sql2 =
    "SELECT iso2, name FROM countries WHERE iso2 IN ({:s})";

  std::map<std::string, std::string> country_names;
  std::set<std::string> countries = get_unique_values<string>(theR, "iso2");
  if (countries.empty())
    return country_names;  // No countries to process

  const std::vector<std::string> language_codes = getLanguageCodes(theOptions.GetLanguage());

  const std::string sqlStmt1 = fmt::format(sql1, quote(countries), quote(language_codes));
  pqxx::result res = conn->executeNonTransaction(sqlStmt1);
  for (const auto& row : res)
  {
    if (row.size() < 2)
      continue;  // Skip rows that do not have the expected columns
    std::string iso2 = row[0].as<std::string>();
    std::string name = row[1].as<std::string>();
    if (!name.empty())
    {
      // If name is already present, keep the shorter one (result is already ordered by length)
      auto it = country_names.find(iso2);
      if (it == country_names.end())
      {
        country_names[iso2] = name;
        countries.erase(iso2);  // Do not need to perform another query for this country
      }
    }
  }

  if (not countries.empty())
  {
    // If there are still countries left, query the countries table
    // to get their names. This is needed for countries that do not
    // have an entry in the geonames table.
    const std::string sqlStmt2 = fmt::format(sql2, quote(countries));
    res = conn->executeNonTransaction(sqlStmt2);
    for (const auto& row : res)
    {
      if (row.size() < 1)
        continue;  // Skip rows that do not have the expected columns
      const std::string iso2 = row[0].as<std::string>();
      const std::string name = row[1].as<std::string>();
      auto it = country_names.find(iso2);
      if (it == country_names.end() and !name.empty())
      {
        country_names[iso2] = iso2;  // Use iso2 as name if no other name found
      }
    }
  }

  return country_names;
}
catch (const Fmi::Exception& e)
{
  std::cerr << e;
  throw;
}

std::map<int, std::string> Query::getMunicipalityNames(
    const QueryOptions& theOptions,
    const pqxx::result& theR)
try
{
  constexpr const char *sql1 = "SELECT id, name FROM municipalities WHERE id IN ({})";

  constexpr const char *sql2 =
     "SELECT"
     "   municipalities_id id, name "
     "FROM"
     "   alternate_municipalities "
     "WHERE municipalities_id IN ({})"
     " AND language IN ({})";

  const bool is_fi = theOptions.GetLanguage() == "fi";
  std::map<int, std::string> municipality_names;
  std::set<int> municipalities = get_unique_values<int>(theR, "municipalities_id");
  const std::vector<std::string> language_codes = getLanguageCodes(theOptions.GetLanguage());

  for (std::set<int>::const_iterator it = municipalities.begin();
       it != municipalities.end(); )
  {
    std::vector<int> currMunicipalities;
    for (; it != municipalities.end() && currMunicipalities.size() < 1000; ++it)
    {
      currMunicipalities.push_back(*it);
    }

    // Query the municipalities table to get the names
    const std::string sqlStmt1 = fmt::format(sql1, quote(currMunicipalities));
    pqxx::result res = conn->executeNonTransaction(sqlStmt1);
    for (const auto& row : res)
    {
      if (row.size() < 2)
        continue;  // Skip rows that do not have the expected columns
      const int id = row[0].as<int>();
      if (row[1].is_null())
        continue;  // Skip rows with null name
      std::string name = row[1].as<std::string>();
      if (!name.empty())
      {
        // If name is already present, keep the shorter one (result is already ordered by length)
        auto it = municipality_names.find(id);
        if (it == municipality_names.end())
        {
          municipality_names[id] = name;
        }
      }
    }
  }

  // FIXME: onko tämä oikea tapa ulkomaanasennusten tapauksessa?
  if (not is_fi)
  {
    for (std::set<int>::const_iterator it = municipalities.begin();
         it != municipalities.end(); )
    {
      std::vector<int> currMunicipalities;
      for (; it != municipalities.end() && currMunicipalities.size() < 1000; ++it)
      {
        currMunicipalities.push_back(*it);
      }

      const std::string sqlStmt2 = fmt::format(sql2, quote(currMunicipalities), quote(language_codes));
      const auto res = conn->executeNonTransaction(sqlStmt2);
      for (const auto& row : res)
      {
        if (row.size() < 1)
          continue;  // Skip rows that do not have the expected columns
        const int id = row[0].as<int>();
        if (row[1].is_null())
          continue;  // Skip rows with null name
        const std::string name = row[1].as<std::string>();
        if (not name.empty())
        {
          municipality_names[id] = name;  // Use id as name if no other name found
        }
      }
    }
  }

  return municipality_names;
}
catch (...)
{
  throw Fmi::Exception::Trace(BCP, "Operation failed");
}


// ----------------------------------------------------------------------
/*!
 * Resolve names for administrative areas (state) which references are present in the result set.
 *
 * \param theOptions Query options
 * \param theR Result set
 * \return mapping of admin area code to its name
 */
// ----------------------------------------------------------------------

std::map<std::string, std::string> Query::getAdministrativeNames(
    const QueryOptions& theOptions,
    const pqxx::result& theR)
{
  constexpr  const char* sql = "SELECT code, name FROM admin1codes WHERE code IN ({})";

  std::map<std::string, std::string> admin_names;

  const std::optional<int> opt_admin1_col = find_column(theR, "admin1");
  const std::optional<int> opt_country_col = find_column(theR, "country_iso2");
  if (!opt_admin1_col || !opt_country_col)
    return admin_names;  // No admin1 or country_iso2 columns found

  const int admin1_col = *opt_admin1_col;
  const int country_col = *opt_country_col;

  // Collect used admin1 codes. Unfortunately in this case we cannot use
  // get_unique_values because we need to combine country_iso2 and admin1
  std::set<std::string> admin_codes;
  for (const auto& row : theR)
  {
    if (row[admin1_col].is_null() or row[country_col].is_null())
      continue;  // Skip rows that do not have the expected columns

    const std::string admin1 = row[admin1_col].as<std::string>();
    const std::string country_iso2 = row[country_col].as<std::string>();
    if (admin1.empty() || country_iso2.empty())
      continue;  // Skip empty admin1 or country_iso2

    const std::string key = country_iso2 + "." + admin1;
    admin_codes.insert(key);
  }

  // Query the admin1codes table to get the names
  // We need to query the admin1codes table in batches to avoid too large queries (total size could be acceptable,
  // but limit however single query to no more than 1000 admin1 codes).
  for (std::set<std::string>::const_iterator it = admin_codes.begin(); it != admin_codes.end(); )
  {
    std::vector<std::string> curr_admin_codes;
    for (; it != admin_codes.end() && curr_admin_codes.size() < 1000; ++it)
    {
      curr_admin_codes.push_back(*it);
    }
    const std::string sqlStmt = fmt::format(sql, quote(curr_admin_codes));
    pqxx::result res = conn->executeNonTransaction(sqlStmt);
    for (const auto& row : res)
    {
      if (row.size() < 2 || row[0].is_null() || row[1].is_null())
        continue;  // Skip rows that do not have the expected columns or id or their values are NULL
      const std::string code = row[0].as<std::string>();
      const std::string name = row[1].as<std::string>();
      if (!name.empty())
      {
        // Use the admin code as key and name as value
        admin_names[*it] = name;
      }
    }
  }

  return admin_names;
}


std::map<int, int> Query::getFmisids(
    const QueryOptions& theOptions,
    const pqxx::result& theR)
try
{
  constexpr const char* sql =
    "SELECT geonames_id, name FROM alternate_geonames "
    "WHERE language='fmisid' AND geonames_id IN ({})";

  const std::set<std::string> ids = get_unique_values<string>(theR, "id");
  std::map<int, int> fmisids;
  for (std::set<std::string>::const_iterator it = ids.begin(); it != ids.end();)
  {
    std::vector<std::string> currIds;
    for (; it != ids.end() and currIds.size() < 1000; )
    {
      currIds.push_back(*it++);
    }

    const std::string sqlStmt = fmt::format(sql, quote(currIds));
    pqxx::result res = conn->executeNonTransaction(sqlStmt);

    // Get the fmisids from the result set
    for (const auto& row : res)
    {
      if (row.size() < 2 || row["name"].is_null())
        continue;  // Skip rows that do not have the expected columns or id

      const int id = row[0].as<int>();
      const auto& field = row[1];
      const int fmisid = field.as<int>();

      fmisids[id] = fmisid;
    }
  }

  return fmisids;
}
catch (...)
{
  throw Fmi::Exception::Trace(BCP, "Operation failed");
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
  try
  {
    return_type locations;

    if (theR.empty())
      return locations;

    // Caches subquery results

    const std::map<int, std::string> name_variants = getNameVariants(theOptions, theR, theSearchWord);
    const std::map<std::string, std::string> country_cache = getCountryNames(theOptions, theR);
    const std::map<int, std::string> municipality_cache = getMunicipalityNames(theOptions, theR);
    const std::map<std::string, std::string> admin_cache = getAdministrativeNames(theOptions, theR);
    const std::map<int, int> fmisids = getFmisids(theOptions, theR);
    const map<string, string> feature_cache = getFeatures(theOptions, theR);

    // Process one location at a time

    for (pqxx::result::const_iterator row = theR.begin(); row != theR.end(); ++row)
    {
      // Do not handle locations without timezones. This is just a safety check,
      // NULL timezones should be removed already in the SQL query, otherwise
      // you might get zero results if the result count limit is 1.

      if (row["timezone"].is_null())
        continue;

      const int id = row["id"].as<int>();
      std::string name = (!row["name"].is_null() ? row["name"].as<string>() : "NULL");

      // Check whether name variant should be used
      auto it1 = name_variants.find(id);
      if (it1 != name_variants.end())
        name = it1->second;

      if ((!row["ansiname"].is_null()) && (theOptions.GetCharset() != "utf8"))
      name = from_utf(name, row["ansiname"].as<string>(), theOptions.GetCharset());

      // Elevation

      int elevation = 0;

      if (!row["elevation"].is_null() && (row["elevation"].as<int>() != 0))
        elevation = row["elevation"].as<int>();
      else if (!row["dem"].is_null())
        elevation = row["dem"].as<int>();

      // Country and description

      string country;
      string iso2;
      if (!row["iso2"].is_null())
      {
        iso2 = row["iso2"].as<string>();
        const auto pos = country_cache.find(iso2);
        if (pos != country_cache.end())
          country = pos->second;
      }

      // Feature code and description

      string description;
      string features_code;
      if (!row["features_code"].is_null())
      {
        features_code = row["features_code"].as<string>();
        const auto pos = feature_cache.find(features_code);
        if (pos != feature_cache.end())
          description = pos->second;
      }

      // Administrative areas

      string administrative;
      if (row["municipalities_id"].is_null())
      {
        // If municipalities_id is NULL, we try to resolve administrative area
        // from admin1 and iso2 fields

        auto admin1 = row["admin1"].as<string>();
        if (!admin1.empty())
        {
          auto localiso2 = row["iso2"].as<string>();
          string key = admin1 + '|' + localiso2;
          const auto pos = admin_cache.find(key);
          if (pos != admin_cache.end())
            administrative = pos->second;
        }
      }
      else
      {
        // If municipalities_id is not NULL, we try to resolve administrative area
        // from municipalities_id field

        const int municipalities_id = row["municipalities_id"].as<int>();
        auto pos = municipality_cache.find(municipalities_id);
        if (pos != municipality_cache.end())
          administrative = pos->second;
      }


      // Check if area is correct
      bool ok = true;

      if (!theArea.empty())
      {
        string lc_area = boost::locale::to_lower(theArea, default_locale);
        ok = (lc_area == boost::locale::to_lower(country, default_locale) ||
              lc_area == boost::locale::to_lower(administrative, default_locale));
      }

      if (ok)
      {
        SimpleLocation loc(name,
                           row["lon"].as<float>(),
                           row["lat"].as<float>(),
                           country,
                           features_code,
                           description,
                           row["timezone"].as<string>(),
                           administrative,
                           row["population"].as<unsigned int>(),
                           iso2,
                           row["id"].as<int>(),
                           elevation);

        const auto fmisid_it = fmisids.find(id);
        if (fmisid_it != fmisids.end())
          loc.fmisid = fmisid_it->second;

        locations.emplace_back(loc);
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

      // Remove "%" from searchword
      string tmp = theSearchWord.substr(0, theSearchWord.size() - 1);

      for (const auto& location : locations)
      {
        if (boost::iequals(tmp, location.name))
          bestmatches.push_back(location);
        else
          secondarymatches.push_back(location);
      }

      locations.clear();

      for (const auto& match : bestmatches)
        locations.push_back(match);

      for (const auto& match : secondarymatches)
        locations.push_back(match);
    }

    return locations;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

string Query::constructSQLStatement(SQLQueryId theQueryId,
                                    const map<SQLQueryParameterId, std::any>& theParams)
{
  try
  {
    std::string sql;

    const auto constructLanguageCodeCondition = [this](const std::string& language) -> std::string
    {
      const std::vector<std::string> codes = get_iso639_table()->get_codes(language);
      if (codes.empty())
        return "=" + conn->quote(language);

      if (codes.size() == 1)
        return "=" + conn->quote(codes.at(0));

      std::string result = " in (";
      for (std::size_t i = 0; i < codes.size(); i++)
      {
        if (i)
          result += ", ";

        result += conn->quote(codes.at(i));
      }
      result += ") ";
      return result;
    };

    const auto& theOptions = std::any_cast<const QueryOptions&>(theParams.at(eQueryOptions));

    switch (theQueryId)
    {
      case eResolveNameVariant:
      {
        auto theGeonamesId = std::any_cast<int>(theParams.at(eGeonamesId));
        auto theSearchWord = std::any_cast<string>(theParams.at(eSearchWord));
        string language = theOptions.GetLanguage();
        Fmi::ascii_tolower(language);

        if (!theOptions.GetAutoCompleteMode())
        {
          sql +=
              "SELECT name,length(name) AS l, priority FROM alternate_geonames WHERE geonames_id=";
          sql += Fmi::to_string(theGeonamesId);
          sql += " AND language" + constructLanguageCodeCondition(language);
          sql +=
              " AND historic=false AND colloquial=false ORDER BY priority ASC, preferred DESC, l "
              "ASC, name ASC LIMIT 1";
        }
        else
        {
          sql +=
              "SELECT name,length(name) As l, priority FROM alternate_geonames WHERE geonames_id=";
          sql += Fmi::to_string(theGeonamesId);
          sql += " AND language" + constructLanguageCodeCondition(language);
          sql += " AND name LIKE ";
          sql += conn->quote(theSearchWord);
          sql +=
              " AND historic=false AND colloquial=false ORDER BY priority ASC, preferred DESC, l "
              "ASC, name ASC LIMIT 1";
        }
        break;
      }
      case eResolveNameVariants:
        {
          auto theGeonamesIds = std::any_cast<std::vector<int>>(theParams.at(eGeonamesId));
          string language = theOptions.GetLanguage();
          Fmi::ascii_tolower(language);

          sql +=
              "SELECT geonames_id, name,length(name) AS l, priority FROM alternate_geonames WHERE ";
          sql += selectByValueCond("geonames_id", theGeonamesIds);
          sql += " AND language" + constructLanguageCodeCondition(language);
          sql +=
              " AND historic=false AND colloquial=false ORDER BY priority ASC, preferred DESC, l "
              "ASC, name ASC";
        }
        break;

      case eFetchByName:
      {
        if (theOptions.GetSearchVariants())
          sql += "(";

        auto theSearchWord = std::any_cast<string>(theParams.at(eSearchWord));
        auto theCountryPriorities = std::any_cast<string>(theParams.at(eCountryPriorities));
        auto theFeaturePriorities = std::any_cast<string>(theParams.at(eFeaturePriorities));

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
        sql += conn->quote(theSearchWord);
        sql += ')';

        if (conn->collateSupported())
        {
          sql += " COLLATE ";
          sql += conn->quote(theOptions.GetCollation());
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
          sql += conn->quote(theSearchWord);
          sql += ")";

          if (conn->collateSupported())
          {
            sql += " COLLATE ";
            sql += conn->quote(theOptions.GetCollation());
          }

          // FIXME: update this
          sql +=
              " AND alternate_geonames.geonames_id=geonames.id AND alternate_geonames.language "
              "LIKE ";
          sql += conn->quote(language);

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
            sql += " AND alternate_geonames.language";
            sql += constructLanguageCodeCondition(language);
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

        if (conn->collateSupported())
        {
          sql += " COLLATE ";
          sql += conn->quote(theOptions.GetCollation());
        }

        break;
      }
      case eFetchByLonLat:
      {
        auto theLongitude = std::any_cast<float>(theParams.at(eLongitude));
        auto theLatitude = std::any_cast<float>(theParams.at(eLatitude));
        auto theRadius = std::any_cast<float>(theParams.at(eRadius));

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
        sql += ")'), the_geog, true) as distance FROM geonames WHERE ";

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
        auto theId = std::any_cast<int>(theParams.at(eGeonameId));

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
        auto theKeyword = std::any_cast<string>(theParams.at(eKeyword));

        if (theQueryId == eFetchByKeyword1)
        {
          sql += "SELECT keyword FROM keywords WHERE keyword=";
          sql += conn->quote(theKeyword);
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
          sql += conn->quote(theKeyword);
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
          sql += conn->quote(theKeyword);
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
              "    AND alternate_municipalities.language";
          sql += constructLanguageCodeCondition(theOptions.GetLanguage());
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
          sql += conn->quote(theKeyword);
          sql += "    AND alternate_geonames.language";
          sql += constructLanguageCodeCondition(theOptions.GetLanguage());
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
              "    AND alternate_geonames.language";
          sql += constructLanguageCodeCondition(theOptions.GetLanguage());
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
        auto theKeyword = std::any_cast<string>(theParams.at(eKeyword));

        sql += "SELECT count(*) AS count FROM keywords_has_geonames WHERE keyword=";
        sql += conn->quote(theKeyword);
        break;
      }
        // With -Weverything complains as all cases are covered
        //    default:
        //      break;
    }

    return sql;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::shared_ptr<const ISO639> Query::get_iso639_table()
{
  std::shared_ptr<ISO639>& iso639 = get_mutable_iso639_table();
  return std::atomic_load(&iso639);
}

std::shared_ptr<ISO639>& Query::get_mutable_iso639_table()
{
  // Initially initialize with empty table
  static std::shared_ptr<ISO639> iso639(new ISO639);
  return iso639;
}

void Query::load_iso639_table(const std::vector<std::string>& special_codes)
{
  std::shared_ptr<ISO639> new_table(new ISO639(*conn, special_codes));
  std::atomic_store(&get_mutable_iso639_table(), new_table);
}

}  // namespace Locus

// ======================================================================
