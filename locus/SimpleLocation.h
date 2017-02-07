// ======================================================================
/*!
 * \brief Interface of class Locus::SimpleLocation
 */
// ======================================================================

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Locus
{
class SimpleLocation
{
 public:
  std::string name;
  float lat;
  float lon;
  std::string country;
  std::string feature;
  std::string description;
  std::string admin;
  std::string timezone;
  unsigned int population;
  std::string iso2;
  int id;
  int elevation;

 public:
  SimpleLocation(const std::string& theName,
                 float theLongitude,
                 float theLatitude,
                 const std::string& theCountry,
                 const std::string& theFeature = "UNKN",
                 const std::string& theDescription = "",
                 const std::string& theTimeZone = "",
                 const std::string& theAdmin = "",
                 int thePopulation = 0,
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
