/* -*- C++ -*- */
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SQLITEDBSVC_ISQLITEDBSVC_H
#define SQLITEDBSVC_ISQLITEDBSVC_H

#include "SQLiteDBSvc/Statement.h"

// Gaudi includes
#include "GaudiKernel/IInterface.h"

// Standard library includes
#include <source_location>  // Improves error messages
#include <string_view>

class ISQLiteDBSvc : virtual public IInterface {
 public:
  DeclareInterfaceID(ISQLiteDBSvc, 1, 0);

  /// Compile a prepared statement attached to this database.
  /// The service will manage the statement object and delete it in its
  /// destructor.
  /// @param[in] statement SQL statement to be compiled
  virtual SQLite::Statement createStatement(
      std::string_view statement,
      std::source_location call = std::source_location::current()) = 0;
};

#endif  // SQLITEDBSVC_ISQLITEDBSVC_H
