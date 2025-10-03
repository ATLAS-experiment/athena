/* -*- C++ -*- */
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SQLiteDBSvc/Statement.h"

#include <format>
#include <stdexcept>

namespace SQLite {
Statement::Statement(sqlite3* db, std::string_view sql,
                     std::source_location call)
    : m_db(db), m_creationPoint(call) {
  const int err =
      sqlite3_prepare_v2(db, sql.data(), int(sql.size()), &m_stmt, nullptr);
  if (err != 0) {
    throw std::logic_error(
        std::format("ERROR preparing SQLite statement: {} ({}) at {} [{}:{}]",
                    sqlite3_errstr(err), sqlite3_errmsg(m_db),
                    m_creationPoint.function_name(),
                    m_creationPoint.file_name(), m_creationPoint.line()));
  }
}

Statement& Statement::operator=(Statement&& rhs) noexcept {
  std::scoped_lock lck(rhs.m_stmtMutex, m_stmtMutex);
  m_stmt = rhs.m_stmt;
  m_db = rhs.m_db;
  m_creationPoint = rhs.m_creationPoint;
  rhs.m_stmt = nullptr;
  rhs.m_db = nullptr;
  return *this;
}

Statement::~Statement() {
  if (m_stmt == nullptr) {
    return;  // Empty statement (unfilled or moved-from)
  }
  sqlite3_finalize(m_stmt);
  m_stmt = nullptr;
}

void Statement::reset() {
  const int err = sqlite3_reset(m_stmt);
  if (err != 0) {
    throw std::runtime_error(
        std::format("ERROR in SQLite statement reset: {} ({})",
                    sqlite3_errstr(err), sqlite3_errmsg(m_db)));
  }
  sqlite3_clear_bindings(m_stmt);
}

bool Statement::step() {
  int err = SQLITE_BUSY;
  while (err == SQLITE_BUSY) {
    err = sqlite3_step(m_stmt);
  }
  switch (err) {
    case SQLITE_DONE:
      return false;
    case SQLITE_ROW:
      return true;
    default:
      throw std::runtime_error(
          std::format("ERROR in SQLite statement step: {} ({})",
                      sqlite3_errstr(err), sqlite3_errmsg(m_db)));
  }
}

// These functions just throw a tuple of the parameter index and error code on
// error This is reinterpreted in the run function
void Statement::bind(int index, std::int64_t value) {
  const int err = sqlite3_bind_int64(m_stmt, index, value);
  if (err != 0) {
    throw std::tuple<int, int>{err, index};
  }
}

void Statement::bind(int index, double value) {
  const int err = sqlite3_bind_double(m_stmt, index, value);
  if (err != 0) {
    throw std::tuple<int, int>{err, index};
  }
}

void Statement::bind(int index, std::string_view value) {
  const int err = sqlite3_bind_text(m_stmt, index, value.data(),
                                    int(value.size()), SQLITE_TRANSIENT);
  if (err != 0) {
    throw std::tuple<int, int>{err, index};
  }
}
}  // namespace SQLite
