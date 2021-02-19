// ======================================================================
/*!
 * \brief Interface of class Locus::QueryOptions
 */
// ======================================================================

#pragma once

#include <list>
#include <map>
#include <string>
#include <boost/optional.hpp>

namespace Locus
{
class QueryOptions
{
 public:
  QueryOptions();

  // Set search conditions

  void SetCountries(const std::string& theCountries);
  void SetCountries(const std::list<std::string>& theCountries);
  void SetExcludedCountries(const std::string& theCountries);
  void SetExcludedCountries(const std::list<std::string>& theCountries);
  void SetResultLimit(unsigned int theLimit);
  void SetFeatures(const std::string& theFeatures);
  void SetFeatures(const std::list<std::string>& theFeatures);
  void SetKeywords(const std::string& theKeywords);
  void SetKeywords(const std::list<std::string>& theKeywords);
  void SetFullCountrySearch(bool theFlag);
  void SetSearchVariants(bool theFlag);
  void SetLanguage(const std::string& theLanguage);
  void SetCharset(const std::string& theCharset);
  void SetCollation(const std::string& theCollation);
  void SetAutoCollation(bool theValue);
  void SetAutocompleteMode(bool theValue);
  void SetPopulationMin(unsigned int theValue);
  void SetPopulationMax(unsigned int theValue);
  void SetNameType(const std::string& theNameType);

  const std::list<std::string>& GetCountries() const { return countries; }
  const std::list<std::string>& GetExcludedCountries() const { return excluded_countries; }
  unsigned int GetResultLimit() const { return result_limit; }
  std::list<std::string> GetFeatures() const { return features; }
  std::list<std::string> GetKeywords() const { return keywords; }
  bool GetFullCountrySearch() const { return fullcountrysearch; }
  bool GetSearchVariants() const { return search_variants; }
  const std::string& GetLanguage() const { return language; }
  const std::string& GetCharset() const { return charset; }
  std::string GetCollation() const;
  unsigned int GetPopulationMin() const { return population_min; }
  unsigned int GetPopulationMax() const { return population_max; }
  const std::string& GetNameType() { return name_type; }
  bool GetAutoCompleteMode() const { return autocompletemode; }
  std::string Hash() const;
  std::size_t HashValue() const;

 private:
  std::list<std::string> countries;           // List of countries to be searched
  bool fullcountrysearch;                     // Search all if above list fails?
  std::string language;                       // Language used in results
  unsigned int result_limit;                  // Limit for the number of results
  std::list<std::string> features;            // Features to search for
  std::list<std::string> keywords;            // Keywords to search for
  bool search_variants;                       // Include variants in search?
  std::string charset;                        // Character set used in result
  unsigned int population_min;                // Search populated places bigger than this
  unsigned int population_max;                // Search populated places smaller than this
  std::list<std::string> excluded_countries;  // Countries that are not used in search
  std::string name_type;                      // Can be 'fmisid','wmo','lpnn' or empty

  std::string collation;  // collation for mysql
  bool autocollation;
  bool autocompletemode;

};  // class QueryOptions

}  // namespace Locus

// ======================================================================
