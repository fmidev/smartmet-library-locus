// ======================================================================
/*!
 * \brief Interface of class Locus::Query
 */
// ======================================================================

#pragma once

#include "ISO639.h"
#include "QueryOptions.h"
#include "SimpleLocation.h"

#include <any>
#include <optional>
#include <memory>
#include <macgyver/PostgreSQLConnection.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeTraits.h>

#include <memory>
#include <pqxx/pqxx>
#include <string>
#include <vector>

namespace Locus
{
class Query
{
 public:
  using return_type = std::vector<SimpleLocation>;
  static const float default_radius;

  ~Query() = default;
  Query() = delete;
  Query(const Query& other) = delete;
  Query& operator=(const Query& other) = delete;
  Query(Query&& other) = delete;
  Query& operator=(Query&& other) = delete;

  Query(const std::string& theHost,
        const std::string& theUser,
        const std::string& thePass,
        const std::string& theDatabase);

  Query(const std::string& theHost,
        const std::string& theUser,
        const std::string& thePass,
        const std::string& theDatabase,
        const std::string& thePort);

  void SetDebug(bool theFlag);

  // Perform the queries
  return_type FetchByName(const QueryOptions& theOptions, const std::string& theName);
  return_type FetchByLatLon(const QueryOptions& theOptions,
                            float theLatitude,
                            float theLongitude,
                            float theRadius = default_radius);
  return_type FetchByLonLat(const QueryOptions& theOptions,
                            float theLongitude,
                            float theLatitude,
                            float theRadius = default_radius);
  return_type FetchById(const QueryOptions& theOptions, int theID);
  return_type FetchByKeyword(const QueryOptions& theOptions, const std::string& theKeyword);
  unsigned int CountKeywordLocations(const QueryOptions& theOptions, const std::string& theKeyword);

  static std::shared_ptr<const ISO639> get_iso639_table();

  void load_iso639_table(
      const std::vector<std::string>& special_codes = std::vector<std::string>());

  void cancel();

 private:
  // Helper methods
  std::string ResolveNameVariant(const QueryOptions& theOptions,
                                 int theId,
                                 const std::string& theSearchWord = "%");
  std::map<int, std::string>
  ResolveNameVariants(const QueryOptions& theOptions,
                      const std::vector<int>& theIds);

  void AddCountryConditions(const QueryOptions& theOptions, std::string& theQuery) const;
  void AddFeatureConditions(const QueryOptions& theOptions, std::string& theQuery) const;
  void AddKeywordConditions(const QueryOptions& theOptions, std::string& theQuery) const;

  return_type build_locations(const QueryOptions& theOptions,
                              const pqxx::result& theR,
                              const std::string& theSearchWord,
                              const std::string& theArea = "");

  std::map<int, std::string> getNameVariants(
      const QueryOptions& theOptions,
      const pqxx::result& theR,
      const std::string& theSearchWord = "%");

  std::map<std::string, std::string> getFeatures(
      const QueryOptions& theOptions,
      const pqxx::result& theR);

  std::map<std::string, std::string> getCountryNames(
      const QueryOptions& theOptions,
      const pqxx::result& theR);

  std::map<int, std::string> getMunicipalityNames(
      const QueryOptions& theOptions,
      const pqxx::result& theR);

  std::map<std::string, std::string> getAdministrativeNames(
    const QueryOptions& theOptions,
    const pqxx::result& theR);

  std::map<int, int> getFmisids(
      const QueryOptions& theOptions,
      const pqxx::result& theR);

  std::vector<std::string> getLanguageCodes(const std::string& language);

  void SetOptions(const QueryOptions& theOptions);

  static std::shared_ptr<ISO639>& get_mutable_iso639_table();

  // ids for queries
  enum SQLQueryId
  {
    eResolveNameVariant,
    eResolveNameVariants,
    eFetchByName,
    eFetchByLonLat,
    eFetchById,
    eFetchByKeyword1,
    eFetchByKeyword2,
    eFetchByKeyword3,
    eCountKeywordLocations
  };

  // ids for query parameters
  enum SQLQueryParameterId
  {
    eQueryOptions,
    eSearchWord,
    eFeatureCode,
    eGeonamesId,
    eCountryIso2Code,
    eMunicipalityId,
    eLocationName,
    eCountryPriorities,
    eFeaturePriorities,
    eLongitude,
    eLatitude,
    eRadius,
    eAdminCode,
    eGeonameId,
    eKeyword
  };

  std::unique_ptr<Fmi::Database::PostgreSQLConnection> conn;  // Location database connecton
  bool debug = false;                                         // Print debug information if true
  bool recursive_query = false;                               // Infinite recursion prevention

  std::string constructSQLStatement(
      SQLQueryId theQueryId,
      const std::map<SQLQueryParameterId,
                     std::any>& theParams);  // construct SQL statement

  template <typename ValueType>
  typename std::enable_if<std::is_same<ValueType, std::string>::value, std::string>::type
  quote(const ValueType& value) const
  {
    return conn->quote(value);
  }

  template <typename ValueType>
  typename std::enable_if<std::is_integral<ValueType>::value, std::string>::type
  quote(const ValueType& value) const
  {
    return Fmi::to_string(value);
  }

  template <typename ContainerType>
  typename std::enable_if<Fmi::is_iterable<ContainerType>::value
     && !std::is_same_v<ContainerType, std::string>, std::string>::type
  quote(const ContainerType& value) const
  {
    std::string result;
    for (const auto& item : value)
    {
      if (!result.empty())
        result += ", ";
      result += quote(item);
    }
    return result;
  }

  template <typename ValueType>
  typename std::enable_if<
    std::is_same_v<ValueType, std::string> || std::is_integral_v<ValueType>,
    std::string>::type
  selectByValueCond(const std::string& column, const ValueType& value)
  {
    return column + "=" + quote(value);
  }

  template <typename ContainerType>
  typename std::enable_if<Fmi::is_iterable<ContainerType>::value, std::string>::type
  selectByValueCond(const std::string& column, const ContainerType& values) const
  {
    if (values.empty())
    {
      return "";
    }
    else
    {
      std::string result = column + " IN (";
      for (const auto& item : values)
      {
        if (!result.empty() && result.back() != '(')
          result += ", ";
        result += quote(item);
      }
      result += ")";
      return result;
    }
  }
};                                             // class Query

}  // namespace Locus

// ======================================================================
