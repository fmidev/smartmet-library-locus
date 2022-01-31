#include "ISO639.h"
#include "Query.h"
#include <regression/tframe.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace Locus;

#ifndef DATABASE_HOST
#define DATABASE_HOST "smartmet-test"
#endif
#ifndef DATABASE_USER
#define DATABASE_USER "fminames_user"
#endif
#ifndef DATABASE_PASS
#define DATABASE_PASS "fminames_pw"
#endif
#ifndef DATABASE_PORT
#define DATABASE_PORT "5444"
#endif
#ifndef DATABASE
#define DATABASE "fminames"
#endif

namespace ISO639Test
{
  void search_by_iso_639_1_name()
  {
    ISO639 iso639;
    const ISO639::Entry e1 = {std::string("fi"), std::string("fin"), "fin", ""};
    const ISO639::Entry e2 = {std::string("fo"), std::string("bar"), "foo", ""};
    iso639.add(e1);
    iso639.add(e2);

    boost::optional<ISO639::Entry> entry;
    entry = iso639.get("fi");
    if (!entry) {
      TEST_FAILED("Did not found language codes for Finland");
    }

    if (entry) {
      //std::cout << *entry << std::endl;
      if (entry->iso639_1) {
	if (*entry->iso639_1 != "fi") {
	  TEST_FAILED("Got ISO 639-1 name '" + *entry->iso639_1 + "' ('fi' expected)");
	}
      } else {
	  TEST_FAILED("ISO 639-1 name missing ('fi' expected)");
      }
    }

    entry = iso639.get("bar");
    if (!entry) {
      TEST_FAILED("Search by ISO 639-2 name failed");
    }

    if (entry) {
      //std::cout << *entry << std::endl;
      if (entry->iso639_1) {
	if (*entry->iso639_1 != "fo") {
	  TEST_FAILED("Got ISO 639-1 name '" + *entry->iso639_1 + "' ('fo' expected)");
	}
      } else {
	  TEST_FAILED("ISO 639-1 name missing ('fo' expected)");
      }

      if (entry->iso639_2) {
	if (*entry->iso639_2 != "bar") {
	  TEST_FAILED("Got ISO 639-2 name '" + *entry->iso639_2 + "' ('bar' expected)");
	}
      } else {
	  TEST_FAILED("ISO 639-2 name missing ('bar' expected)");
      }
    }
   
    TEST_PASSED();
  }

  void load_from_geonames_and_search()
  {
    Query lq(DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE, DATABASE_PORT);

    boost::optional<ISO639::Entry> entry;

    entry = lq.get_iso639_table()->get("lv");
    if (entry) {
      TEST_FAILED("Language codes not yet loaded");
    }

    lq.load_iso639_table({"wmo", "fmisid", "lpnn"});
    
    entry = lq.get_iso639_table()->get("lv");
    if (!entry) {
      TEST_FAILED("Did not found language codes for Latvia");
    } else {
      //std::cout << *entry << std::endl;
      if (!entry->iso639_1 || (*entry->iso639_1 != "lv")
	  || (!entry->iso639_2 || (*entry->iso639_2 != "lav"))
	  || (entry->iso639_3 != "lav")) {
	std::ostringstream tmp;
	tmp << *entry;
	TEST_FAILED("Returned language codes " + tmp.str() + " differs from expected ones");
      }
    }

// Houm: tietokannassa ovat ISO 639-2/T koodit. Pitäisikö olla ISO 639-2/B koodeja?
//       https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
// Sen vuoksi osa tarkistuksia alhaalla ovat kommntoittu pois
    
    entry = lq.get_iso639_table()->get("de");
    if (!entry) {
      TEST_FAILED("Searching 'ger' should return language codes for Germany");
    } else {
      //std::cout << *entry << std::endl;
      if (!entry->iso639_1 || (*entry->iso639_1 != "de")
//	  || (!entry->iso639_2 || (*entry->iso639_2 != "ger"))
	  || (entry->iso639_3 != "deu")) {
	std::ostringstream tmp;
	tmp << *entry;
	TEST_FAILED("Returned language codes " + tmp.str() + " differs from expected ones");
      }

//      const ISO639::Entry* entry2 = lq.get_iso639_table()->get("ger");
//      if (entry2 != entry) {
//	TEST_FAILED("Searching 'ger' should also return language codes for Germany");
//    }

      boost::optional<ISO639::Entry> entry3 = lq.get_iso639_table()->get("deu");
      if (!entry3 || (entry3->iso639_3 != entry->iso639_3)) {
	TEST_FAILED("Searching 'deu' should also return language codes for Germany");
      }
    }

    entry = lq.get_iso639_table()->get("fmisid");
    if (!entry) {
      TEST_FAILED("Searching for fmisid should return special codes");
    } else {
      if (entry->iso639_1
	  || entry->iso639_2
	  || (entry->iso639_3 != "fmisid")) {
	std::ostringstream tmp;
	tmp << *entry;
	TEST_FAILED("Returned language codes " + tmp.str() + " differs from expected ones");
      }
    }
    
    TEST_PASSED();
  }
  
  // The actual test driver
  class tests : public tframe::tests
  {
    //! Overridden message separator
    virtual const char* error_message_prefix() const { return "\n\t"; }
    //! Main test suite
    void test(void)
    {
      TEST(search_by_iso_639_1_name);
      TEST(load_from_geonames_and_search);
    }
  };

}  // namespace QueryTest

int main(void)
{
  cout << endl
       << "ISO639 (languge codes) tester" << endl
       << "=============================" << endl;
  ISO639Test::tests t;
  return t.run();
}

