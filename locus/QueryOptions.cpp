// ======================================================================
/*!
 * \brief Implementation of class Locus::QueryOptions
 */
// ======================================================================

#include "QueryOptions.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <macgyver/Exception.h>
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
  try
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
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    if (!theCountries.empty())
      boost::algorithm::split(countries, theCountries, boost::algorithm::is_any_of(","));
    else
      countries.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    countries = theCountries;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    if (!theCountries.empty())
      boost::algorithm::split(excluded_countries, theCountries, boost::algorithm::is_any_of(","));
    else
      excluded_countries.clear();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    excluded_countries = theCountries;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    result_limit = theLimit;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    if (!theFeatures.empty())
      boost::algorithm::split(features, theFeatures, boost::algorithm::is_any_of(","));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    features = theFeatures;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    if (!theKeywords.empty())
      boost::algorithm::split(keywords, theKeywords, boost::algorithm::is_any_of(","));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    keywords = theKeywords;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Set the character set
 */
// ----------------------------------------------------------------------

void QueryOptions::SetCharset(const string& theCharset)
{
  try
  {
    charset = theCharset;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    collation = theCollation;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    autocollation = theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Get collation string
 */
// ----------------------------------------------------------------------

string QueryOptions::GetCollation() const
{
  try
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
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    autocompletemode = theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    fullcountrysearch = theFlag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    search_variants = theFlag;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    language = boost::algorithm::to_lower_copy(theLanguage);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    population_min = theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    population_max = theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    name_type = theNameType;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
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
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return unique hash value based on the options
 */
// ----------------------------------------------------------------------

std::size_t QueryOptions::HashValue() const
{
  try
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
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Locus

// ======================================================================
