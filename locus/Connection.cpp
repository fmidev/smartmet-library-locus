#include "Connection.h"
#include <macgyver/Exception.h>
#include <boost/make_shared.hpp>
#include <iostream>

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif
const std::string default_port = "5432";
#ifdef __llvm__
#pragma clang diagnostic pop
#endif

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
  try
  {
    open(theHost, theUser, thePass, theDatabase, theClientEncoding);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
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
  try
  {
    open(theHost, theUser, thePass, theDatabase, theClientEncoding, thePort);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Connection::Connection(const std::string& theHost,
                       const std::string& theUser,
                       const std::string& thePass,
                       const std::string& theDatabase,
                       const std::string& theClientEncoding,
                       const std::string& thePort,
                       const std::string& theConnectTimeout /* = "" */,
                       bool theDebug /*= false*/)
    : debug(theDebug), collate(false)
{
  try
  {
    open(theHost, theUser, thePass, theDatabase, theClientEncoding, thePort, theConnectTimeout);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool Connection::open(const std::string& theHost,
                      const std::string& theUser,
                      const std::string& thePass,
                      const std::string& theDatabase,
                      const std::string& theClientEncoding)
{
  try
  {
    return open(theHost, theUser, thePass, theDatabase, theClientEncoding, default_port);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool Connection::open(const std::string& theHost,
                      const std::string& theUser,
                      const std::string& thePass,
                      const std::string& theDatabase,
                      const std::string& /* theClientEncoding */,
                      const std::string& thePort,
                      const std::string& theConnectTimeout /*= ""*/)
{
  try
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
    // clang-format on

    if (!theConnectTimeout.empty())
      ss << " connect_timeout=" << theConnectTimeout;

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
      throw Fmi::Exception(BCP,"Failed to connect to " + theUser + "@" + theDatabase + ":" + thePort +
                               " : " + e.what());
    }

    return conn->is_open();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Connection::close()
{
  try
  {
    if (!conn)
      return;

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
      throw Fmi::Exception(BCP,string("Failed to close connection to PostgreSQL: ") + e.what());
    }
#endif
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::string Connection::quote(const std::string& theString) const
{
  try
  {
    if (conn)
      return conn->quote(theString);

    throw Fmi::Exception(BCP,"Locus: Attempting to quote string without database connection");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

pqxx::result Connection::executeNonTransaction(const std::string& theSQLStatement) const
{
  try
  {
    if (debug)
      std::cout << "SQL: " << theSQLStatement << std::endl;

    try
    {
      pqxx::nontransaction ntrx(*conn);
      return ntrx.exec(theSQLStatement);
    }
    catch (const std::exception& e)
    {
      throw Fmi::Exception(BCP,std::string("Execution of SQL statement failed: ").append(e.what()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Connection::startTransaction()
{
  try
  {
    trx = boost::make_shared<pqxx::work>(*conn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

pqxx::result Connection::executeTransaction(const std::string& theSQLStatement) const
{
  try
  {
    if (debug)
      std::cout << "SQL: " << theSQLStatement << std::endl;

    try
    {
      return trx->exec(theSQLStatement);
    }
    catch (const std::exception& e)
    {
      throw Fmi::Exception(BCP,
          std::string("Execution of SQL statement (transaction mode) failed: ").append(e.what()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Connection::commitTransaction()
{
  try
  {
    try
    {
      trx->commit();
      trx.reset();
    }
    catch (const std::exception& e)
    {
      // If we get here, Xaction has been rolled back
      trx.reset();
      throw Fmi::Exception(BCP,std::string("Commiting transaction failed: ").append(e.what()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Connection::setClientEncoding(const std::string& theEncoding) const
{
  try
  {
    try
    {
      conn->set_client_encoding(theEncoding);
    }
    catch (const std::exception& e)
    {
      throw Fmi::Exception(BCP,std::string("set_client_encoding failed: ").append(e.what()));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Locus

// ======================================================================
