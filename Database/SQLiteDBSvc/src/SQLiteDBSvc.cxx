/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SQLiteDBSvc.h"

StatusCode SQLiteDBSvc::initialize() {
  sqlite3* temp = nullptr;
  const int err = sqlite3_open_v2(m_databasePath.value().c_str(), &temp,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                      SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_URI,
                                  nullptr);
  if (err != 0) {
    ATH_MSG_ERROR("Error opening SQLite DB: << " << sqlite3_errstr(err));
    return StatusCode::FAILURE;
  }
  m_db.reset(temp);
  ATH_MSG_INFO("Opened db connection to " << m_databasePath.value().c_str());
  return StatusCode::SUCCESS;
}

SQLite::Statement SQLiteDBSvc::createStatement(std::string_view statement,
                                               std::source_location call) {
  return SQLite::Statement(m_db.get(), statement, call);
}
