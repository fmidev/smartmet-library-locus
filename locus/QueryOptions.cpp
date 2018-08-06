// ======================================================================
/*!
 * \brief Implementation of class Locus::QueryOptions
 */
// ======================================================================

#include "QueryOptions.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/functional/hash.hpp>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace Locus
{
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

QueryOptions::QueryOptions()
    : countries(),
      fullcountrysearch(false),
      language("fi"),
      result_limit(100),
      features(),
      search_variants(true),
      charset("utf8"),
      population_min(0),
      population_max(0),
      excluded_countries(),
      collation("utf8_general_ci"),
      autocollation(false),
      autocompletemode(false)
{
  // Remaining initializations

  countries.push_back("fi");
  features.push_back("PPLC");
  features.push_back("ADMD");
  features.push_back("PPLA");
  features.push_back("PPLA2");
  features.push_back("PPLA3");
  features.push_back("PPLG");
  features.push_back("PPL");
  features.push_back("ADM2");
  features.push_back("ISL");
  features.push_back("PPLX");
  features.push_back("POST");
  features.push_back("SKI");
}

// ----------------------------------------------------------------------
/*!
 * Set countries used in search. Takes list of country codes in importance
 * order separated with comma. Special value 'all' can be used to define
 * all countries to be searched.
 *
 * \param value List of iso country codes (string) separated with comma.
 * \return void
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
 * \param value List of iso country codes
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCountries(const list<string>& theCountries) { countries = theCountries; }
// ----------------------------------------------------------------------
/*!
 * Set countries that are not used in search. Takes list of country
 * codes in importance order separated with comma.
 * Special value 'all' can be used to define all countries to be searched.
 *
 * \param value List of iso country codes (string) separated with comma.
 * \return void
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
 * \param value List of iso country codes separated with comma.
 * \return void
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
 * \param value Maximum number of results
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetResultLimit(unsigned int theLimit) { result_limit = theLimit; }
// ----------------------------------------------------------------------
/*!
 * Set features used in search. Takes list of feature codes in importance
 * order separated with comma. Special value 'all' can be used to define
 * all features.
 *
 * \param features List of iso features codes (string) separated with comma.
 * \return void
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
 * \param features List of iso features codes
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetFeatures(const list<string>& theFeatures) { features = theFeatures; }
// ----------------------------------------------------------------------
/*!
 * Set keywords used in search.
 *
 * \param keywords List of iso keywords separated with comma
 * \return void
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
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetKeywords(const list<std::string>& theKeywords) { keywords = theKeywords; }
// ----------------------------------------------------------------------
/*!
 * \brief Set the character set
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCharset(const string& theCharset) { charset = theCharset; }
// ----------------------------------------------------------------------
/*!
 * Set collation. Collations that mysql supports for utf-8 are valid.
 * Collation defines how string comparisons are made and how results
 * are sorted
 *
 * \param collation Mysql collate string
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCollation(const string& theCollation) { collation = theCollation; }
// ----------------------------------------------------------------------
/*!
 * Set autocollation. Autocollation allows collation setting defined
 * automatically based on language. Autocollation doesn't support all
 * languages.
 *
 * \param value true/false
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetAutoCollation(bool theValue) { autocollation = theValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Get collation string
 */
// ----------------------------------------------------------------------

string QueryOptions::GetCollation() const
{
  if (autocollation)
  {
    if (language == "fi")
      return "utf8_swedish_ci";
    else if (language == "sv")
      return "utf8_swedish_ci";
    else if (language == "et")
      return "utf8_estonian_ci";
    else
      return "utf8_general_ci";
  }
  else
    return collation;
}

// ----------------------------------------------------------------------
/*!
 * Set autocompletemode. Autocompletemode returns only names that match
 * exactly to searchword. This is not normally required. For example
 * search Villmanstrand may return Lappeenranta
 *
 * \param value true/false
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetAutocompleteMode(bool theValue) { autocompletemode = theValue; }
// ----------------------------------------------------------------------
/*!
 * To purpose of this method was to search all countries if search
 * with specified countries fails. Usually search with 'all'
 * is fast enough and method is usually useless
 *
 * \param value Boolean true or false
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetFullCountrySearch(bool theFlag) { fullcountrysearch = theFlag; }
// ----------------------------------------------------------------------
/*!
 * Defines should variants be searched. This is on by default
 *
 * \param value Boolean true or false
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetSearchVariants(bool theFlag) { search_variants = theFlag; }
// ----------------------------------------------------------------------
/*!
 * Language for results. For example if language 'fi' is set then
 * result 'Praha' is returned when searching 'Prague'.
 *
 * \param value ISO language code
 * \return void
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
 * \param value Population limit
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetPopulationMin(unsigned int theValue) { population_min = theValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Search populated places smaller than this
 *
 * \param value Population limit
 * \return void
 */
// ----------------------------------------------------------------------

void QueryOptions::SetPopulationMax(unsigned int theValue) { population_max = theValue; }
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
       << ':' << autocollation << ':' << autocompletemode << ':';

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
  std::size_t hash = boost::hash_value(fullcountrysearch);
  boost::hash_combine(hash, boost::hash_value(language));
  boost::hash_combine(hash, boost::hash_value(result_limit));
  boost::hash_combine(hash, boost::hash_value(search_variants));
  boost::hash_combine(hash, boost::hash_value(charset));
  boost::hash_combine(hash, boost::hash_value(population_min));
  boost::hash_combine(hash, boost::hash_value(population_max));
  boost::hash_combine(hash, boost::hash_value(collation));
  boost::hash_combine(hash, boost::hash_value(autocollation));
  boost::hash_combine(hash, boost::hash_value(autocompletemode));

  for (const string& c : countries)
    boost::hash_combine(hash, boost::hash_value(c));
  for (const string& f : features)
    boost::hash_combine(hash, boost::hash_value(f));
  for (const string& k : keywords)
    boost::hash_combine(hash, boost::hash_value(k));
  for (const string& c : excluded_countries)
    boost::hash_combine(hash, boost::hash_value(c));

  return hash;
}

}  // namespace Locus

// ======================================================================
