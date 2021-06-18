// ======================================================================
/*!
 * \brief Interface of class Locus::QueryOptions
 */
// ======================================================================

#pragma once

#include <boost/optional.hpp>
#include <list>
#include <map>
#include <string>

namespace Locus
{
class QueryOptions
{
 public:
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
  std::list<std::string> features{"PPLC",
                                  "ADMD",
                                  "PPLA",
                                  "PPLA2",
                                  "PPLA3",
                                  "PPLG",
                                  "PPL",
                                  "ADM2",
                                  "ISL",
                                  "PPLX",
                                  "POST",
                                  "AIRP",
                                  "HBR",
                                  "SKI",
                                  "MT",
                                  "MTS",
                                  "PRK"};     // Features to search for
  std::list<std::string> countries{"fi"};     // List of countries to be searched
  std::list<std::string> keywords;            // Keywords to search for
  std::list<std::string> excluded_countries;  // Countries that are not used in search
  std::string language = "fi";                // Language used in results
  std::string charset = "utf8";               // Character set used in result
  std::string name_type;                      // Can be 'fmisid','wmo','lpnn' or empty
  unsigned int result_limit = 100;            // Limit for the number of results
  unsigned int population_max = 0;            // Search populated places smaller than this
  unsigned int population_min = 0;            // Search populated places bigger than this
  bool fullcountrysearch = false;             // Search all if above list fails?
  bool search_variants = true;                // Include variants in search?

  std::string collation = "utf8_general_ci";  // collation for mysql
  bool autocollation = false;
  bool autocompletemode = false;

};  // class QueryOptions

}  // namespace Locus

// ======================================================================
