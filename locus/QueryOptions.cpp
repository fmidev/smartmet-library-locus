// ======================================================================
/*!
 * \brief Implementation of class Locus::QueryOptions
 */
// ======================================================================

#include "QueryOptions.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <macgyver/Hash.h>
#include <sstream>
#include <stdexcept>

using std::list;
using std::ostringstream;
using std::string;

namespace Locus
{
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

QueryOptions::QueryOptions()
    : fullcountrysearch(false),
      language("fi"),
      result_limit(100),
      search_variants(true),
      charset("utf8"),
      population_min(0),
      population_max(0),
      collation("utf8_general_ci"),
      autocollation(false),
      autocompletemode(false)
{
  // Remaining initializations

  countries.emplace_back("fi");
  features.emplace_back("PPLC");
  features.emplace_back("ADMD");
  features.emplace_back("PPLA");
  features.emplace_back("PPLA2");
  features.emplace_back("PPLA3");
  features.emplace_back("PPLG");
  features.emplace_back("PPL");
  features.emplace_back("ADM2");
  features.emplace_back("ISL");
  features.emplace_back("PPLX");
  features.emplace_back("POST");
  features.emplace_back("AIRP");
  features.emplace_back("HBR");
  features.emplace_back("SKI");
  features.emplace_back("MT");
  features.emplace_back("MTS");
  features.emplace_back("PRK");
}

// ----------------------------------------------------------------------
/*!
 * Set countries used in search. Takes list of country codes in importance
 * order separated with comma. Special value 'all' can be used to define
 * all countries to be searched.
 *
 * \param theCountries List of iso country codes (string) separated with comma.
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCountries(const string& theCountries)
{
  if (!theCountries.empty())
    boost::algorithm::split(countries, theCountries, boost::algorithm::is_any_of(","));
  else
    countries.clear();
}

// ----------------------------------------------------------------------
/*!
 * Set countries used in search. Takes list of country codes in importance,
 * Special value 'all' can be used to define all countries to be searched.
 *
 * \param theCountries List of iso country codes
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCountries(const list<string>& theCountries)
{
  countries = theCountries;
}
// ----------------------------------------------------------------------
/*!
 * Set countries that are not used in search. Takes list of country
 * codes in importance order separated with comma.
 * Special value 'all' can be used to define all countries to be searched.
 *
 * \param theCountries List of iso country codes (string) separated with comma.
 */
// ----------------------------------------------------------------------

void QueryOptions::SetExcludedCountries(const string& theCountries)
{
  if (!theCountries.empty())
    boost::algorithm::split(excluded_countries, theCountries, boost::algorithm::is_any_of(","));
  else
    excluded_countries.clear();
}

// ----------------------------------------------------------------------
/*!
 * Set countries that are not used in search. Takes list of country
 * codes in importance order.
 * Special value 'all' can be used to define all countries to be searched.
 *
 * \param theCountries List of iso country codes separated with comma.
 */
// ----------------------------------------------------------------------

void QueryOptions::SetExcludedCountries(const list<string>& theCountries)
{
  excluded_countries = theCountries;
}

// ----------------------------------------------------------------------
/*!
 * Set limit for result rows to not overheat servers when using wildcards
 * or common names like Springfield in searches
 *
 * \param theLimit Maximum number of results
 */
// ----------------------------------------------------------------------

void QueryOptions::SetResultLimit(unsigned int theLimit)
{
  result_limit = theLimit;
}
// ----------------------------------------------------------------------
/*!
 * Set features used in search. Takes list of feature codes in importance
 * order separated with comma. Special value 'all' can be used to define
 * all features.
 *
 * \param theFeatures List of iso features codes (string) separated with comma.
 */
// ----------------------------------------------------------------------

void QueryOptions::SetFeatures(const string& theFeatures)
{
  if (!theFeatures.empty())
    boost::algorithm::split(features, theFeatures, boost::algorithm::is_any_of(","));
}

// ----------------------------------------------------------------------
/*!
 * Set features used in search. Takes list of feature codes in importance
 * order. Special value 'all' can be used to define all features.
 *
 * \param theFeatures List of iso features codes
 */
// ----------------------------------------------------------------------

void QueryOptions::SetFeatures(const list<string>& theFeatures)
{
  features = theFeatures;
}
// ----------------------------------------------------------------------
/*!
 * Set keywords used in search.
 *
 * \param theKeywords List of iso keywords separated with comma
 */
// ----------------------------------------------------------------------

void QueryOptions::SetKeywords(const string& theKeywords)
{
  if (!theKeywords.empty())
    boost::algorithm::split(keywords, theKeywords, boost::algorithm::is_any_of(","));
}

// ----------------------------------------------------------------------
/*!
 * Set keywords used in search. Takes list of keywords.
 *
 * \param theKeywords List of keywords
 */
// ----------------------------------------------------------------------

void QueryOptions::SetKeywords(const list<std::string>& theKeywords)
{
  keywords = theKeywords;
}
// ----------------------------------------------------------------------
/*!
 * \brief Set the character set
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCharset(const string& theCharset)
{
  charset = theCharset;
}
// ----------------------------------------------------------------------
/*!
 * Set collation. Collations that mysql supports for utf-8 are valid.
 * Collation defines how string comparisons are made and how results
 * are sorted
 *
 * \param theCollation Mysql collate string
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCollation(const string& theCollation)
{
  collation = theCollation;
}
// ----------------------------------------------------------------------
/*!
 * Set autocollation. Autocollation allows collation setting defined
 * automatically based on language. Autocollation doesn't support all
 * languages.
 *
 * \param theValue true/false
 */
// ----------------------------------------------------------------------

void QueryOptions::SetAutoCollation(bool theValue)
{
  autocollation = theValue;
}
// ----------------------------------------------------------------------
/*!
 *�\brief Get collation string
 */
// ----------------------------------------------------------------------

string QueryOptions::GetCollation() const
{
  if (!autocollation)
    return collation;
  if (language == "fi")
    return "utf8_swedish_ci";
  if (language == "sv")
    return "utf8_swedish_ci";
  if (language == "et")
    return "utf8_estonian_ci";
  return "utf8_general_ci";
}

// ----------------------------------------------------------------------
/*!
 * Set autocompletemode. Autocompletemode returns only names that match
 * exactly to searchword. This is not normally required. For example
 * search Villmanstrand may return Lappeenranta
 *
 * \param theValue true/false
 */
// ----------------------------------------------------------------------

void QueryOptions::SetAutocompleteMode(bool theValue)
{
  autocompletemode = theValue;
}
// ----------------------------------------------------------------------
/*!
 * To purpose of this method was to search all countries if search
 * with specified countries fails. Usually search with 'all'
 * is fast enough and method is usually useless
 *
 * \param theFlag Boolean true or false
 */
// ----------------------------------------------------------------------

void QueryOptions::SetFullCountrySearch(bool theFlag)
{
  fullcountrysearch = theFlag;
}
// ----------------------------------------------------------------------
/*!
 * Defines should variants be searched. This is on by default
 *
 * \param theFlag Boolean true or false
 */
// ----------------------------------------------------------------------

void QueryOptions::SetSearchVariants(bool theFlag)
{
  search_variants = theFlag;
}
// ----------------------------------------------------------------------
/*!
 * Language for results. For example if language 'fi' is set then
 * result 'Praha' is returned when searching 'Prague'.
 *
 * \param theLanguage ISO language code
 */
// ----------------------------------------------------------------------

void QueryOptions::SetLanguage(const string& theLanguage)
{
  language = boost::algorithm::to_lower_copy(theLanguage);
}

// ----------------------------------------------------------------------
/*!
 * \brief Search populated places bigger than this
 *
 * \param theValue Population limit
 */
// ----------------------------------------------------------------------

void QueryOptions::SetPopulationMin(unsigned int theValue)
{
  population_min = theValue;
}
// ----------------------------------------------------------------------
/*!
 * \brief Search populated places smaller than this
 *
 * \param theValue Population limit
 */
// ----------------------------------------------------------------------

void QueryOptions::SetPopulationMax(unsigned int theValue)
{
  population_max = theValue;
}
// ----------------------------------------------------------------------
/*!
 * \brief Name type can be 'fmisid','wmo','lpnn' or empty
 *
 * \param theNameType Type of name
 */
// ----------------------------------------------------------------------

void QueryOptions::SetNameType(const std::string& theNameType)
{
  name_type = theNameType;
}

// ----------------------------------------------------------------------
/*!
 * Return unique hash value based on the options
 *
 * Deprecated! Use HashValue instead!
 */
// ----------------------------------------------------------------------

string QueryOptions::Hash() const
{
  ostringstream hash;

  hash << fullcountrysearch << ':' << language << ':' << result_limit << ':' << search_variants
       << ':' << charset << ':' << population_min << ':' << population_max << ':' << collation
       << ':' << autocollation << ':' << autocompletemode << ':' << name_type << ':';

  for (const string& c : countries)
    hash << c << ':';
  for (const string& f : features)
    hash << f << ':';
  for (const string& c : excluded_countries)
    hash << c << ':';

  return hash.str();
}

// ----------------------------------------------------------------------
/*!
 * Return unique hash value based on the options
 */
// ----------------------------------------------------------------------

std::size_t QueryOptions::HashValue() const
{
  std::size_t hash = Fmi::hash_value(fullcountrysearch);
  Fmi::hash_combine(hash, Fmi::hash_value(language));
  Fmi::hash_combine(hash, Fmi::hash_value(result_limit));
  Fmi::hash_combine(hash, Fmi::hash_value(search_variants));
  Fmi::hash_combine(hash, Fmi::hash_value(charset));
  Fmi::hash_combine(hash, Fmi::hash_value(population_min));
  Fmi::hash_combine(hash, Fmi::hash_value(population_max));
  Fmi::hash_combine(hash, Fmi::hash_value(collation));
  Fmi::hash_combine(hash, Fmi::hash_value(autocollation));
  Fmi::hash_combine(hash, Fmi::hash_value(autocompletemode));
  Fmi::hash_combine(hash, Fmi::hash_value(name_type));

  for (const string& c : countries)
    Fmi::hash_combine(hash, Fmi::hash_value(c));
  for (const string& f : features)
    Fmi::hash_combine(hash, Fmi::hash_value(f));
  for (const string& k : keywords)
    Fmi::hash_combine(hash, Fmi::hash_value(k));
  for (const string& c : excluded_countries)
    Fmi::hash_combine(hash, Fmi::hash_value(c));

  return hash;
}

}  // namespace Locus

// ======================================================================
