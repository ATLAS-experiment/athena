/* -*- C++ -*- */
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SQLITEDBSVC_SQLITEDBSVC_H
#define SQLITEDBSVC_SQLITEDBSVC_H
#include "AthenaBaseComps/AthService.h"
#include "SQLiteDBSvc/ISQLiteDBSvc.h"

// STL includes
#include <memory>

// Other includes
#include "sqlite3.h"

/** @class SQLiteDBSvc
 *  @brief A service to manage a connection to an SQLite DB
 *
 */
class SQLiteDBSvc : public extends<AthService, ISQLiteDBSvc> {
 public:
  /// Constructor
  SQLiteDBSvc(const std::string& name, ISvcLocator* svcLoc)
      : extends(name, svcLoc), m_db(nullptr, sqlite3_close_v2) {}
  /// Initialize
  virtual StatusCode initialize() override;

  /// Compile a prepared statement attached to this database.
  /// The service will manage the statement object and delete it in finalize.
  /// Statements are protected by a mutex and may be used from multiple threads.
  /// It's reasonable to construct all the prepared statements you might need in
  /// a service or algorithm `initialize`.
  /// @param[in] statement SQL statement to be compiled
  virtual SQLite::Statement createStatement(
      std::string_view statement,
      std::source_location call = std::source_location::current()) override;

 private:
  // Thread-safe because SQLite is used in serialized mode
  std::unique_ptr<sqlite3, int (*)(sqlite3*)> m_db ATLAS_THREAD_SAFE;
  Gaudi::Property<std::string> m_databasePath{
      this, "DatabasePath", ":memory:",
      "Path to SQLite Database. The default (:memory:) creates a temporary "
      "in-memory database. Options may be set using SQLite URI filenames."};
};
#endif  // SQLITEDBSVC_SQLITEDBSVC_H
