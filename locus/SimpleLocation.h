// ======================================================================
/*!
 * \brief Interface of class Locus::SimpleLocation
 */
// ======================================================================

#pragma once

#include <boost/optional.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace Locus
{
class SimpleLocation
{
 public:
  std::string name;
  float lat = 0.0;
  float lon = 0.0;
  std::string country;
  std::string feature;
  std::string description;
  std::string admin;
  std::string timezone;
  unsigned int population;
  std::string iso2;
  int id = 0;
  int elevation = 0;
  boost::optional<int> fmisid;

  SimpleLocation(std::string theName,
                 float theLongitude,
                 float theLatitude,
                 std::string theCountry,
                 std::string theFeature = "UNKN",
                 std::string theDescription = "",
                 std::string theTimeZone = "",
                 std::string theAdmin = "",
                 unsigned int thePopulation = 0,
                 std::string theIso2 = "",
                 int theId = 0,
                 int theElevation = 0)
      : name(std::move(theName)),
        lat(theLatitude),
        lon(theLongitude),
        country(std::move(theCountry)),
        feature(std::move(theFeature)),
        description(std::move(theDescription)),
        admin(std::move(theAdmin)),
        timezone(std::move(theTimeZone)),
        population(thePopulation),
        iso2(std::move(theIso2)),
        id(theId),
        elevation(theElevation)
  {
  }
};  // class SimpleLocation

}  // namespace Locus

// ======================================================================
