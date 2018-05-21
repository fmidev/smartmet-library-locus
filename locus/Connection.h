// ======================================================================
/*!
 * \brief Interface of class Locus::Connection
 */
// ======================================================================

#pragma once

#include <boost/shared_ptr.hpp>
#include <list>
#include <pqxx/pqxx>
#include <string>

namespace Locus
{
class Connection
{
 public:
  ~Connection() { close(); }
  Connection(bool theDebug = false) : debug(theDebug), collate(false) {}
  Connection(const std::string& theHost,
             const std::string& theUser,
             const std::string& thePass,
             const std::string& theDatabase,
             const std::string& theClientEncoding,
             bool theDebug = false);

  Connection(const std::string& theHost,
             const std::string& theUser,
             const std::string& thePass,
             const std::string& theDatabase,
             const std::string& theClientEncoding,
             const std::string& thePort,
             bool theDebug = false);

  Connection(const std::string& theHost,
             const std::string& theUser,
             const std::string& thePass,
             const std::string& theDatabase,
             const std::string& theClientEncoding,
             const std::string& thePort,
             const std::string& theConnectTimeout = "",
             bool theDebug = false);

  bool open(const std::string& theHost,
            const std::string& theUser,
            const std::string& thePass,
            const std::string& theDatabase,
            const std::string& theClientEncoding);

  bool open(const std::string& theHost,
            const std::string& theUser,
            const std::string& thePass,
            const std::string& theDatabase,
            const std::string& theClientEncoding,
            const std::string& thePort,
            const std::string& theConnectTimeout = "");

  void close();
  bool isConnected() const { return conn->is_open(); }
  void setClientEncoding(const std::string& theEncoding) const;
  void setDebug(bool doDebugging) { debug = doDebugging; }
  pqxx::result executeNonTransaction(const std::string& theSQLStatement) const;
  bool collateSupported() { return collate; }
  std::string quote(const std::string& theString) const;

 private:
  boost::shared_ptr<pqxx::connection> conn;  // PostgreSQL connecton
  bool debug;
  bool collate;

};  // class Connection

}  // namespace Locus

// ======================================================================
