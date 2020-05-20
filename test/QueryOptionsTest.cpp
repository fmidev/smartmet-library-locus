#include "QueryOptions.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace boost;

namespace QueryOptionsTest
{
// ----------------------------------------------------------------------

void hash()
{
  Locus::QueryOptions options;

  string result =
      "0:fi:100:1:utf8:0:0:utf8_general_ci:0:0:fi:PPLC:ADMD:PPLA:PPLA2:PPLA3:PPLG:PPL:ADM2:ISL:"
      "PPLX:POST:AIRP:HBR:SKI:MT:MTS:PRK:";

  if (options.Hash() != result)
    TEST_FAILED("Default hash should be " + result + ", not " + options.Hash());

  TEST_PASSED();
}

// ----------------------------------------------------------------------

// The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void) { TEST(hash); }
};  // class tests

}  // namespace QueryOptionsTest

int main(void)
{
  cout << endl << "QueryOptions tester" << endl << "==============================" << endl;
  QueryOptionsTest::tests t;
  return t.run();
}
