// ======================================================================
/*!
 * \brief Interface of class Locus::Query
 */
// ======================================================================

#pragma once

#include "QueryOptions.h"
#include "SimpleLocation.h"
#include "ISO639.h"

#include <macgyver/PostgreSQLConnection.h>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

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

  boost::shared_ptr<const ISO639> get_iso639_table();

  void load_iso639_table(const std::vector<std::string>& special_codes = std::vector<std::string>());

  void cancel();

 private:
  // Helper methods
  boost::optional<int> ResolveFmisid(const QueryOptions& theOptions, const std::string& theId);
  std::string ResolveFeature(const QueryOptions& theOptions, const std::string& theCode);
  std::string ResolveNameVariant(const QueryOptions& theOptions,
                                 const std::string& theId,
                                 const std::string& theSearchWord = "%");
  std::string ResolveCountry(const QueryOptions& theOptions, const std::string& theIsoCode);
  std::string ResolveMunicipality(const QueryOptions& theOptions, const std::string& theId);
  std::string ResolveAdministrative(const std::string& theCode, const std::string& theCountry);
  void AddCountryConditions(const QueryOptions& theOptions, std::string& theQuery) const;
  void AddFeatureConditions(const QueryOptions& theOptions, std::string& theQuery) const;
  void AddKeywordConditions(const QueryOptions& theOptions, std::string& theQuery) const;

  return_type build_locations(const QueryOptions& theOptions,
                              const pqxx::result& theR,
                              const std::string& theSearchWord,
                              const std::string& theArea = "");

  void SetOptions(const QueryOptions& theOptions);

  static boost::shared_ptr<ISO639>& get_mutable_iso639_table();

  // ids for queries
  enum SQLQueryId
  {
    eResolveFeature,
    eResolveNameVariant,
    eResolveCountry1,
    eResolveCountry2,
    eResolveMunicipality1,
    eResolveMunicipality2,
    eResolveAdministrative,
    eResolveFmisid,
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

  std::unique_ptr<Fmi::Database::PostgreSQLConnection> conn;       // Location database connecton
  bool debug = false;            // Print debug information if true
  bool recursive_query = false;  // Infinite recursion prevention

  std::string constructSQLStatement(
      SQLQueryId theQueryId,
      const std::map<SQLQueryParameterId,
                     boost::any>& theParams);  // construct SQL statement
};                                             // class Query

}  // namespace Locus

// ======================================================================
