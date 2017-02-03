#include "Connection.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

const std::string default_port = "5432";

using namespace std;

namespace Locus
{
Connection::Connection(const std::string& theHost,
                       const std::string& theUser,
                       const std::string& thePass,
                       const std::string& theDatabase,
                       const std::string& theClientEncoding,
                       bool theDebug /*= false*/)
    : debug(theDebug), collate(false)
{
  open(theHost, theUser, thePass, theDatabase, theClientEncoding);
}

Connection::Connection(const std::string& theHost,
                       const std::string& theUser,
                       const std::string& thePass,
                       const std::string& theDatabase,
                       const std::string& theClientEncoding,
                       const std::string& thePort,
                       bool theDebug /*= false*/)
    : debug(theDebug), collate(false)
{
  open(theHost, theUser, thePass, theDatabase, theClientEncoding, thePort);
}

bool Connection::open(const std::string& theHost,
                      const std::string& theUser,
                      const std::string& thePass,
                      const std::string& theDatabase,
                      const std::string& theClientEncoding)
{
  return open(theHost, theUser, thePass, theDatabase, theClientEncoding, default_port);
}

bool Connection::open(const std::string& theHost,
                      const std::string& theUser,
                      const std::string& thePass,
                      const std::string& theDatabase,
                      const std::string& theClientEncoding,
                      const std::string& thePort)
{
  close();

  std::stringstream ss;

  // clang-format off
  ss << "host="      << theHost
	 << " dbname="   << theDatabase
	 << " port= "    << thePort
	 << " user="     << theUser
	 << " password=" << thePass
#if 0
	 << " client_encoding=" << theClientEncoding
#endif
	;
	// clang-format off

  try
  {
    conn = boost::make_shared<pqxx::connection>(ss.str());
    /*
      if(PostgreSQL > 9.1)
      collateSupported = true;
      pqxx::result res = executeNonTransaction("SELECT version()");
    */
  }
  catch (const std::exception& e)
  {
    throw runtime_error(string("Failed to connect to FmiNames database: ") + e.what());
  }

  return conn->is_open();
}

void Connection::close()
{
  if (!conn) return;
  conn->disconnect();

#if 0
	// disconnect does not throw according to documentation
	try
	  {
		if(conn->is_open())
		  conn->disconnect();
	  }
	catch(const std::exception& e)
	  {
		throw runtime_error(string("Failed to close connection to PostgreSQL: ") + e.what());
	  }
#endif
}

std::string Connection::quote(const std::string& theString) const
{
  if (conn)
    return conn->quote(theString);
  else
    throw std::runtime_error("Locus: Attempting to quote string without database connection");
}

pqxx::result Connection::executeNonTransaction(const std::string& theSQLStatement) const
{
  if (debug) std::cout << "SQL: " << theSQLStatement << std::endl;

  try
  {
    pqxx::nontransaction ntrx(*conn);
    return ntrx.exec(theSQLStatement);
  }
  catch (const std::exception& e)
  {
    throw runtime_error(string("Execution of SQL statement failed: ") + e.what());
  }
}

void Connection::setClientEncoding(const std::string& theEncoding) const
{
  try
  {
    conn->set_client_encoding(theEncoding);
  }
  catch (const std::exception& e)
  {
    throw runtime_error(string("set_client_encoding failed: ") + e.what());
  }
}

}  // namespace Locus

// ======================================================================
