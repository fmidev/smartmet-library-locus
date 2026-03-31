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
#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <macgyver/Join.h>
#include <macgyver/StringConversion.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace
{
constexpr const char* CLIENT_ENCODING = "UTF8";

// ----------------------------------------------------------------------
/*!
 * \brief Default locale
 */
// ----------------------------------------------------------------------

const boost::locale::generator locale_generator;
const std::locale default_locale = locale_generator("fi_FI.UTF-8");

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

#ifdef UNUSED
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
#endif

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

std::optional<int> find_column(const pqxx::result& theResult, const std::string& theColumnName)
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
std::set<ValueType> get_unique_values(const pqxx::result& theResult,
                                      const std::string& theColumnName)
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

template <typename T>
std::vector<std::vector<T>> make_batches(const std::set<T>& items, std::size_t batch_size)
{
  std::vector<std::vector<T>> batches;
  std::vector<T> current;
  for (const auto& item : items)
  {
    current.push_back(item);
    if (current.size() >= batch_size)
    {
      batches.push_back(std::move(current));
      current.clear();
    }
  }
  if (!current.empty())
    batches.push_back(std::move(current));
  return batches;
}

}  // namespace

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

std::map<int, std::string> Query::ResolveNameVariants(const QueryOptions& theOptions,
                                                      const vector<int>& theIds)
{
  map<SQLQueryParameterId, std::any> params;
  params[eQueryOptions] = theOptions;

  std::map<int, std::string> retval;
  auto it = theIds.cbegin();
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
      auto name = row[1].as<string>();

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
      std::vector<std::string> upper_countries;
      for (auto c : countries)
      {
        Fmi::ascii_toupper(c);
        upper_countries.push_back(std::move(c));
      }
      theQuery += " AND geonames.countries_iso2 IN (" + quote(upper_countries) + ")";
    }

    const list<string>& excluded_countries = theOptions.GetExcludedCountries();
    if (!excluded_countries.empty() && !contains(countries, "%") && !contains(countries, "all"))
    {
      std::vector<std::string> lower_excluded;
      for (auto c : excluded_countries)
      {
        Fmi::ascii_tolower(c);
        lower_excluded.push_back(std::move(c));
      }
      theQuery += " AND geonames.countries_iso2 NOT IN (" + quote(lower_excluded) + ")";
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
    const list<string>& features = theOptions.GetFeatures();
    if (features.empty() || contains(features, "%") || contains(features, "all"))
      return;
    theQuery += " AND features_code IN (" + quote(features) + ")";
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
    theQuery +=
        " AND geonames.id IN (SELECT geonames_id FROM keywords_has_geonames WHERE keyword IN (" +
        quote(keywords) + "))";
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

    // Set country priorities
    params[eCountryPriorities] =
        buildCasePriority("countries_iso2", theOptions.GetCountries(), "country_priority");

    // Set feature priorities
    params[eFeaturePriorities] =
        buildCasePriority("features_code", theOptions.GetFeatures(), "feature_priority");

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
        cout << "Do full country seach because limited search didn't return results\n";

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

    return build_locations(theOptions, res, "", "");
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

std::map<int, std::string> Query::getNameVariants(const QueryOptions& theOptions,
                                                  const pqxx::result& theR,
                                                  const string& theSearchWord)
{
  std::map<int, std::string> name_variants;
  std::vector<int> variant_resolve_postponed;

  const auto override_field_ind = find_column(theR, "override_name");

  for (pqxx::result::const_iterator row = theR.begin(); row != theR.end(); ++row)
  {
    if (row["timezone"].is_null())
      continue;

    const auto id = row["id"].as<int>();

    // Use override name if available
    if (override_field_ind && !row[*override_field_ind].is_null())
    {
      const auto altname = row[*override_field_ind].as<string>();
      if (!altname.empty())
      {
        name_variants[id] = altname;
        continue;
      }
    }

    // Search for possible translations
    if (theOptions.GetLanguage().empty())
      continue;

    if (theOptions.GetAutoCompleteMode())
    {
      const string variant = ResolveNameVariant(theOptions, id, theSearchWord);
      if (!variant.empty())
        name_variants[id] = variant;
    }
    else
    {
      variant_resolve_postponed.push_back(id);
    }
  }

  // Resolve postponed name variants in batch
  if (!variant_resolve_postponed.empty())
  {
    const auto variants = ResolveNameVariants(theOptions, variant_resolve_postponed);
    for (const auto& [id, name] : variants)
    {
      if (!name.empty() && name_variants.count(id) == 0)
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

std::map<std::string, std::string> Query::getFeatures(const QueryOptions& /* theOptions */,
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
    auto code = row[0].as<std::string>();
    auto shortdesc = row[1].as<std::string>();
    if (!shortdesc.empty())
    {
      features[code] = shortdesc;
    }
  }
  return features;
}

std::map<std::string, std::string> Query::getCountryNames(const QueryOptions& theOptions,
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

  constexpr const char* sql2 = "SELECT iso2, name FROM countries WHERE iso2 IN ({:s})";

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
    auto iso2 = row[0].as<std::string>();
    auto name = row[1].as<std::string>();
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
      const auto iso2 = row[0].as<std::string>();
      const auto name = row[1].as<std::string>();
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

void Query::fetchFinnishMunicipalityNames(std::map<int, std::string>& names,
                                           const std::vector<int>& ids) const
{
  constexpr const char* sql = "SELECT id, name FROM municipalities WHERE id IN ({})";
  pqxx::result res = conn->executeNonTransaction(fmt::format(sql, quote(ids)));
  for (const auto& row : res)
  {
    if (row.size() < 2 || row[1].is_null())
      continue;
    const int id = row[0].as<int>();
    const auto name = row[1].as<std::string>();
    if (!name.empty())
      names.try_emplace(id, name);
  }
}

void Query::fetchAlternateMunicipalityNames(std::map<int, std::string>& names,
                                             const std::vector<int>& ids,
                                             const std::vector<std::string>& language_codes) const
{
  constexpr const char* sql =
      "SELECT municipalities_id id, name FROM alternate_municipalities"
      " WHERE municipalities_id IN ({}) AND language IN ({})";
  pqxx::result res =
      conn->executeNonTransaction(fmt::format(sql, quote(ids), quote(language_codes)));
  for (const auto& row : res)
  {
    if (row.size() < 2 || row[1].is_null())
      continue;
    const int id = row[0].as<int>();
    const auto name = row[1].as<std::string>();
    if (!name.empty())
      names[id] = name;  // Override with translated name
  }
}

std::map<int, std::string> Query::getMunicipalityNames(const QueryOptions& theOptions,
                                                       const pqxx::result& theR)
try
{
  const bool is_fi = theOptions.GetLanguage() == "fi";
  std::map<int, std::string> municipality_names;
  const std::set<int> municipalities = get_unique_values<int>(theR, "municipalities_id");
  const std::vector<std::string> language_codes = getLanguageCodes(theOptions.GetLanguage());

  for (const auto& batch : make_batches(municipalities, 1000))
  {
    fetchFinnishMunicipalityNames(municipality_names, batch);
    if (!is_fi)
      fetchAlternateMunicipalityNames(municipality_names, batch, language_codes);
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
    const QueryOptions& /* theOptions */, const pqxx::result& theR)
{
  constexpr const char* sql = "SELECT code, name FROM admin1codes WHERE code IN ({})";

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

    const auto admin1 = row[admin1_col].as<std::string>();
    const auto country_iso2 = row[country_col].as<std::string>();
    if (admin1.empty() || country_iso2.empty())
      continue;  // Skip empty admin1 or country_iso2

    const std::string key = country_iso2 + "." + admin1;
    admin_codes.insert(key);
  }

  // Query the admin1codes table to get the names
  // We need to query the admin1codes table in batches to avoid too large queries (total size could
  // be acceptable, but limit however single query to no more than 1000 admin1 codes).
  for (auto it = admin_codes.begin(); it != admin_codes.end();)
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
      const auto code = row[0].as<std::string>();
      const auto name = row[1].as<std::string>();
      if (!name.empty())
      {
        // Use the admin code as key and name as value
        admin_names[code] = name;
      }
    }
  }

  return admin_names;
}

std::map<int, int> Query::getFmisids(const QueryOptions& /* theOptions */, const pqxx::result& theR)
try
{
  constexpr const char* sql =
      "SELECT geonames_id, name FROM alternate_geonames "
      "WHERE language='fmisid' AND geonames_id IN ({})";

  const std::set<std::string> ids = get_unique_values<string>(theR, "id");
  std::map<int, int> fmisids;
  for (auto it = ids.begin(); it != ids.end();)
  {
    std::vector<std::string> currIds;
    for (; it != ids.end() and currIds.size() < 1000;)
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

std::optional<SimpleLocation> Query::buildSingleLocation(
    const QueryOptions& opts,
    const pqxx::result::const_iterator& row,
    const std::map<int, std::string>& name_variants,
    const std::map<std::string, std::string>& country_cache,
    const std::map<int, std::string>& municipality_cache,
    const std::map<std::string, std::string>& admin_cache,
    const std::map<int, int>& fmisids,
    const std::map<std::string, std::string>& feature_cache,
    const std::string& theArea)
{
  if (row["timezone"].is_null())
    return std::nullopt;

  const int id = row["id"].as<int>();
  string name = (!row["name"].is_null() ? row["name"].as<string>() : "NULL");

  const auto it1 = name_variants.find(id);
  if (it1 != name_variants.end())
    name = it1->second;

  if (!row["ansiname"].is_null() && opts.GetCharset() != "utf8")
    name = from_utf(name, row["ansiname"].as<string>(), opts.GetCharset());

  int elevation = 0;
  if (!row["elevation"].is_null() && row["elevation"].as<int>() != 0)
    elevation = row["elevation"].as<int>();
  else if (!row["dem"].is_null())
    elevation = row["dem"].as<int>();

  string country;
  string iso2;
  if (!row["iso2"].is_null())
  {
    iso2 = row["iso2"].as<string>();
    const auto pos = country_cache.find(iso2);
    if (pos != country_cache.end())
      country = pos->second;
  }

  string description;
  string features_code;
  if (!row["features_code"].is_null())
  {
    features_code = row["features_code"].as<string>();
    const auto pos = feature_cache.find(features_code);
    if (pos != feature_cache.end())
      description = pos->second;
  }

  // Resolve administrative area name
  string administrative;
  if (!row["municipalities_id"].is_null())
  {
    const auto pos = municipality_cache.find(row["municipalities_id"].as<int>());
    if (pos != municipality_cache.end())
      administrative = pos->second;
  }
  else if (const auto admin1 = row["admin1"].as<string>(); !admin1.empty())
  {
    const auto pos = admin_cache.find(iso2 + "." + admin1);
    if (pos != admin_cache.end())
      administrative = pos->second;
  }

  if (!theArea.empty())
  {
    const string lc_area = boost::locale::to_lower(theArea, default_locale);
    if (lc_area != boost::locale::to_lower(country, default_locale) &&
        lc_area != boost::locale::to_lower(administrative, default_locale))
      return std::nullopt;
  }

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

  return loc;
}

void Query::sortByExactMatch(return_type& locations, const std::string& theSearchWord)
{
  // Remove trailing "%" from searchword
  const string tmp = theSearchWord.substr(0, theSearchWord.size() - 1);

  return_type bestmatches;
  return_type secondarymatches;
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

Query::return_type Query::build_locations(const QueryOptions& theOptions,
                                          const pqxx::result& theR,
                                          const string& theSearchWord,
                                          const string& theArea /* = ""*/)
{
  try
  {
    if (theR.empty())
      return {};

    const auto name_variants = getNameVariants(theOptions, theR, theSearchWord);
    const auto country_cache = getCountryNames(theOptions, theR);
    const auto municipality_cache = getMunicipalityNames(theOptions, theR);
    const auto admin_cache = getAdministrativeNames(theOptions, theR);
    const auto fmisids = getFmisids(theOptions, theR);
    const auto feature_cache = getFeatures(theOptions, theR);

    return_type locations;
    for (pqxx::result::const_iterator row = theR.begin(); row != theR.end(); ++row)
    {
      auto loc = buildSingleLocation(theOptions, row, name_variants, country_cache,
                                     municipality_cache, admin_cache, fmisids, feature_cache,
                                     theArea);
      if (loc)
      {
        locations.emplace_back(std::move(*loc));
        if (theOptions.GetResultLimit() > 0 && locations.size() >= theOptions.GetResultLimit())
          break;
      }
    }

    if (theOptions.GetAutoCompleteMode())
      sortByExactMatch(locations, theSearchWord);

    return locations;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string Query::languageCodeCondition(const std::string& language) const
{
  const std::vector<std::string> codes = get_iso639_table()->get_codes(language);
  if (codes.empty())
    return "=" + conn->quote(language);
  if (codes.size() == 1)
    return "=" + conn->quote(codes.front());
  return " IN (" + quote(codes) + ")";
}

std::string Query::buildCasePriority(const std::string& column,
                                      const std::list<std::string>& items,
                                      const std::string& alias) const
{
  if (items.size() <= 1)
    return "";
  std::string result = ", CASE " + column;
  int n = 1;
  for (const auto& item : items)
    result += " WHEN " + conn->quote(item) + " THEN " + Fmi::to_string(n++);
  return result + " ELSE 1000 END AS " + alias;
}

std::string Query::buildResolveNameVariantSQL(
    const QueryOptions& opts,
    const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theGeonamesId = std::any_cast<int>(params.at(eGeonamesId));
  const auto theSearchWord = std::any_cast<string>(params.at(eSearchWord));
  string language = opts.GetLanguage();
  Fmi::ascii_tolower(language);

  std::string sql =
      "SELECT name,length(name) AS l, priority FROM alternate_geonames WHERE geonames_id=";
  sql += Fmi::to_string(theGeonamesId);
  sql += " AND language" + languageCodeCondition(language);
  if (opts.GetAutoCompleteMode())
    sql += " AND name LIKE " + conn->quote(theSearchWord);
  sql +=
      " AND historic=false AND colloquial=false ORDER BY priority ASC, preferred DESC, l ASC, name ASC LIMIT 1";
  return sql;
}

std::string Query::buildResolveNameVariantsSQL(
    const QueryOptions& opts,
    const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theGeonamesIds = std::any_cast<std::vector<int>>(params.at(eGeonamesId));
  string language = opts.GetLanguage();
  Fmi::ascii_tolower(language);

  std::string sql =
      "SELECT geonames_id, name,length(name) AS l, priority FROM alternate_geonames WHERE ";
  sql += selectByValueCond("geonames_id", theGeonamesIds);
  sql += " AND language" + languageCodeCondition(language);
  sql +=
      " AND historic=false AND colloquial=false ORDER BY priority ASC, preferred DESC, l ASC, name ASC";
  return sql;
}

std::string Query::buildFetchByNameSQL(const QueryOptions& opts,
                                        const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theSearchWord = std::any_cast<string>(params.at(eSearchWord));
  const auto theCountryPriorities = std::any_cast<string>(params.at(eCountryPriorities));
  const auto theFeaturePriorities = std::any_cast<string>(params.at(eFeaturePriorities));

  auto addPopulationConditions = [&](std::string& sql)
  {
    if (opts.GetPopulationMin() > 0)
      sql += " AND population>=" + Fmi::to_string(opts.GetPopulationMin());
    if (opts.GetPopulationMax() > 0)
      sql += " AND population<=" + Fmi::to_string(opts.GetPopulationMax());
  };

  auto addCollation = [&](std::string& sql)
  {
    if (conn->collateSupported())
      sql += " COLLATE " + conn->quote(opts.GetCollation());
  };

  auto buildSelect = [&](bool withVariants) -> std::string
  {
    std::string sql =
        "SELECT DISTINCT geonames.name AS name,"
        " geonames.ansiname AS ansiname,"
        " lat, lon, countries_iso2 AS iso2,"
        " features_code, timezone, geonames.id as id, geonames.priority as geonames_priority,"
        " municipalities_id, admin1, population,"
        " elevation, dem,"
        " CASE WHEN population>50000 THEN population ELSE 0 END AS population_priority ";
    sql += theCountryPriorities;
    sql += ' ';
    sql += theFeaturePriorities;

    if (!withVariants)
    {
      sql += " FROM geonames WHERE LOWER(geonames.name) LIKE LOWER(";
      sql += conn->quote(theSearchWord);
      sql += ')';
      addCollation(sql);
      sql += " AND timezone IS NOT NULL";
    }
    else
    {
      string language = opts.GetLanguage();
      Fmi::ascii_tolower(language);
      sql += " FROM geonames, alternate_geonames WHERE LOWER(alternate_geonames.name) LIKE LOWER(";
      sql += conn->quote(theSearchWord);
      sql += ")";
      addCollation(sql);
      sql += " AND alternate_geonames.geonames_id=geonames.id AND alternate_geonames.language LIKE ";
      sql += conn->quote(language);
      if (opts.GetAutoCompleteMode())
      {
        sql += " AND alternate_geonames.language";
        sql += languageCodeCondition(language);
      }
    }

    addPopulationConditions(sql);
    AddFeatureConditions(opts, sql);
    AddCountryConditions(opts, sql);
    AddKeywordConditions(opts, sql);
    return sql;
  };

  std::string sql;
  if (opts.GetSearchVariants())
    sql = "(" + buildSelect(false) + ") UNION (" + buildSelect(true) + ")";
  else
    sql = buildSelect(false);

  sql += " ORDER BY geonames_priority, population_priority DESC, ";
  sql += (theCountryPriorities.empty() ? "" : "country_priority, ");
  sql += (theFeaturePriorities.empty() ? "" : "feature_priority, ");
  sql += " population DESC, name";
  if (conn->collateSupported())
    sql += " COLLATE " + conn->quote(opts.GetCollation());

  return sql;
}

std::string Query::buildFetchByLonLatSQL(
    const QueryOptions& opts,
    const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theLongitude = std::any_cast<float>(params.at(eLongitude));
  const auto theLatitude = std::any_cast<float>(params.at(eLatitude));
  const auto theRadius = std::any_cast<float>(params.at(eRadius));

  const std::string point =
      Fmi::to_string(theLongitude) + " " + Fmi::to_string(theLatitude);

  std::string sql =
      "WITH candidates AS ("
      "SELECT geonames.id AS id, geonames.name AS name,"
      "geonames.ansiname AS ansiname, lat, lon,"
      "countries_iso2 AS iso2, features_code, timezone,"
      "population, elevation, dem, municipalities_id,"
      "admin1, ST_Distance(ST_GeographyFromText('POINT(";
  sql += point;
  sql += ")'), the_geog, true) as distance FROM geonames WHERE timezone IS NOT NULL";

  if (opts.GetPopulationMin() > 0)
    sql += " AND population>=" + Fmi::to_string(opts.GetPopulationMin());
  if (opts.GetPopulationMax() > 0)
    sql += " AND population<=" + Fmi::to_string(opts.GetPopulationMax());

  AddCountryConditions(opts, sql);
  AddFeatureConditions(opts, sql);
  AddKeywordConditions(opts, sql);

  sql += " ORDER BY the_geom <-> ST_GeomFromText('POINT(" + point + ")',4326)";

  const int limit_safety_margin = 10;
  if (opts.GetResultLimit() > 0)
    sql += " LIMIT " + Fmi::to_string(limit_safety_margin + opts.GetResultLimit());

  sql += ") SELECT * from candidates";
  if (theRadius > 0)
    sql += " WHERE distance<=" + Fmi::to_string(theRadius * 1000);
  sql += " ORDER BY distance";
  if (opts.GetResultLimit() > 0)
    sql += " LIMIT " + Fmi::to_string(opts.GetResultLimit());

  return sql;
}

std::string Query::buildFetchByIdSQL(
    const std::map<SQLQueryParameterId, std::any>& params)
{
  const auto theId = std::any_cast<int>(params.at(eGeonameId));
  return "SELECT id, name, ansiname, lat, lon, countries_iso2 AS iso2, features_code,"
         " timezone, municipalities_id, admin1, population,"
         " elevation, dem"
         " FROM geonames WHERE id=" +
         Fmi::to_string(theId);
}

std::string Query::buildFetchByKeywordSQL(
    SQLQueryId queryId,
    const QueryOptions& /* opts */,
    const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theKeyword = std::any_cast<string>(params.at(eKeyword));

  if (queryId == eFetchByKeyword1)
    return "SELECT keyword FROM keywords WHERE keyword=" + conn->quote(theKeyword);

  // eFetchByKeyword2
  return "SELECT geonames.id AS id, geonames.name AS name,\n "
         "geonames.ansiname AS ansiname,lat,lon,\n "
         "countries_iso2 AS iso2, features_code, timezone,\n "
         "population, elevation, dem,\n "
         "municipalities_id, admin1,\n "
         "keywords_has_geonames.name AS override_name FROM \n "
         "geonames, keywords_has_geonames WHERE\n "
         "keywords_has_geonames.keyword=" +
         conn->quote(theKeyword) +
         " AND geonames.id=keywords_has_geonames.geonames_id"
         " ORDER BY name";
}

std::string Query::buildCountKeywordLocationsSQL(
    const std::map<SQLQueryParameterId, std::any>& params) const
{
  const auto theKeyword = std::any_cast<string>(params.at(eKeyword));
  return "SELECT count(*) AS count FROM keywords_has_geonames WHERE keyword=" +
         conn->quote(theKeyword);
}

string Query::constructSQLStatement(SQLQueryId theQueryId,
                                    const map<SQLQueryParameterId, std::any>& theParams)
{
  try
  {
    const auto& opts = std::any_cast<const QueryOptions&>(theParams.at(eQueryOptions));
    switch (theQueryId)
    {
      case eResolveNameVariant:
        return buildResolveNameVariantSQL(opts, theParams);
      case eResolveNameVariants:
        return buildResolveNameVariantsSQL(opts, theParams);
      case eFetchByName:
        return buildFetchByNameSQL(opts, theParams);
      case eFetchByLonLat:
        return buildFetchByLonLatSQL(opts, theParams);
      case eFetchById:
        return buildFetchByIdSQL(theParams);
      case eFetchByKeyword1:
      case eFetchByKeyword2:
        return buildFetchByKeywordSQL(theQueryId, opts, theParams);
      case eCountKeywordLocations:
        return buildCountKeywordLocationsSQL(theParams);
    }
    return {};
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
