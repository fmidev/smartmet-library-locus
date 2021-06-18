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

  SimpleLocation(const std::string& theName,
                 float theLongitude,
                 float theLatitude,
                 const std::string& theCountry,
                 const std::string& theFeature = "UNKN",
                 const std::string& theDescription = "",
                 const std::string& theTimeZone = "",
                 const std::string& theAdmin = "",
                 unsigned int thePopulation = 0,
                 const std::string& theIso2 = "",
                 int theId = 0,
                 int theElevation = 0)
      : name(theName),
        lat(theLatitude),
        lon(theLongitude),
        country(theCountry),
        feature(theFeature),
        description(theDescription),
        admin(theAdmin),
        timezone(theTimeZone),
        population(thePopulation),
        iso2(theIso2),
        id(theId),
        elevation(theElevation)
  {
  }
};  // class SimpleLocation

}  // namespace Locus

// ======================================================================
