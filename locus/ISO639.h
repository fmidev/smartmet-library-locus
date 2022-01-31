#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <macgyver/PostgreSQLConnection.h>

namespace Locus
{

/**
 *   @brief Provides mapping of ISO 639 language code (all 3 types)
 *
 *   See https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
 */
class ISO639
{
 public:
  struct Entry
  {
    boost::optional<std::string> iso639_1;
    boost::optional<std::string> iso639_2;
    std::string iso639_3;
    std::string name;
  };

  ISO639() = default;

  ISO639(Fmi::Database::PostgreSQLConnection& conn,
	 const std::vector<std::string>& special_codes = std::vector<std::string>());

  virtual ~ISO639() = default;

  void add(const Entry& entry);
  void add_special_code(const std::string& code);

  boost::optional<Entry> get(const std::string& name) const;

 private:
  std::map<std::string, const Entry*> iso639_1_map;
  std::map<std::string, const Entry*> iso639_2_map;
  std::map<std::string, Entry> iso639_3_map;
};

std::ostream& operator << (std::ostream& os, const ISO639::Entry& entry);
  
}
