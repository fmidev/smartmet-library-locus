#include "ISO639.h"
#include <boost/regex.hpp>
#include <macgyver/Exception.h>

Locus::ISO639::ISO639(Fmi::Database::PostgreSQLConnection& conn,
                      const std::vector<std::string>& special_codes)
{
  std::string sql = "select iso_639_1, iso_639_2, iso_639_3, name from languages";
  pqxx::result res = conn.executeNonTransaction(sql);

  for (pqxx::result::const_iterator row = res.begin(); row != res.end(); ++row)
  {
    Entry entry;
    entry.iso639_3 = row["iso_639_3"].as<std::string>();
    entry.name = row["name"].as<std::string>();
    const auto& c1 = row["iso_639_1"];
    if (!c1.is_null())
    {
      entry.iso639_1 = c1.as<std::string>();
    }
    const auto& c2 = row["iso_639_2"];
    if (!c2.is_null())
    {
      entry.iso639_2 = c2.as<std::string>();
    }
    try
    {
      add(entry);
    }
    catch (const std::exception& e)
    {
      std::cout << "Error reading language codes: " << entry << ": " << e.what() << std::endl;
    }
  }

  for (const std::string& code : special_codes)
  {
    add_special_code(code);
  }
}

void Locus::ISO639::add(const Locus::ISO639::Entry& entry)
{
  const boost::regex r_c2("[a-z][a-z]");
  const boost::regex r_c3("[a-z][a-z][a-z]");
  if (!boost::regex_match(entry.iso639_3, r_c3))
  {
    throw Fmi::Exception(BCP, "Invalid ISO 639-3 language code " + entry.iso639_3);
  }

  auto i3 = iso639_3_map.emplace(entry.iso639_3, entry);
  if (!i3.second)
  {
    throw Fmi::Exception(BCP, "Duplicate ISO 639-3 language code " + entry.iso639_3);
  }
  const Locus::ISO639::Entry* current = &i3.first->second;
  if (entry.iso639_1)
  {
    if (!boost::regex_match(*entry.iso639_1, r_c2))
    {
      throw Fmi::Exception(BCP, "Invalid ISO 639-1 language code " + *entry.iso639_1);
    }
    auto i1 = iso639_1_map.emplace(*entry.iso639_1, current);
    if (!i1.second)
    {
      throw Fmi::Exception(BCP, "Duplicate ISO 639-1 language code " + *entry.iso639_1);
    }
  }

  if (entry.iso639_2 && (*entry.iso639_2 != entry.iso639_3))
  {
    if (!boost::regex_match(*entry.iso639_2, r_c3))
    {
      throw Fmi::Exception(BCP, "Invalid ISO 639-2 language code " + *entry.iso639_2);
    }
    auto i2 = iso639_2_map.emplace(*entry.iso639_2, current);
    if (!i2.second)
    {
      throw Fmi::Exception(BCP, "Duplicate ISO 639-2 language code " + *entry.iso639_2);
    }
  }
}

void Locus::ISO639::add_special_code(const std::string& code)
{
  Entry entry;
  entry.iso639_3 = code;
  iso639_3_map.emplace(code, entry);
}

boost::optional<Locus::ISO639::Entry> Locus::ISO639::get(const std::string& name) const
{
  if (name.length() == 2)
  {
    auto it = iso639_1_map.find(name);
    if (it == iso639_1_map.end())
      return boost::none;

    return *it->second;
  }

  if (name.length() < 3)
    return boost::none;

  auto it1 = iso639_3_map.find(name);
  if (it1 != iso639_3_map.end())
    return it1->second;

  auto it2 = iso639_2_map.find(name);
  if (it2 == iso639_2_map.end())
    return boost::none;

  return *it2->second;
}

std::ostream& Locus::operator<<(std::ostream& os, const Locus::ISO639::Entry& entry)
{
  os << R"("LanguageCodes": { "iso_639-3": ")" << entry.iso639_3 << '"';
  if (entry.iso639_1)
  {
    os << R"(, "iso_639-1": ")" << *entry.iso639_1 << "\"";
  }
  if (entry.iso639_2)
  {
    os << R"(, "iso_639-2": ")" << *entry.iso639_2 << "\"";
  }
  os << R"(, "description": ")" << entry.name << "\"}";
  return os;
}

std::vector<std::string> Locus::ISO639::get_codes(const std::string& name) const
{
  std::vector<std::string> result;
  boost::optional<Entry> entry = get(name);
  if (entry)
  {
    result.push_back(entry->iso639_3);
    if (entry->iso639_2 && (entry->iso639_2 != entry->iso639_3))
    {
      result.push_back(*entry->iso639_2);
    }
    if (entry->iso639_1)
    {
      result.push_back(*entry->iso639_1);
    }
  }
  else
  {
    // FIXME: do we need this
    result.push_back(name);
  }
  return result;
}
