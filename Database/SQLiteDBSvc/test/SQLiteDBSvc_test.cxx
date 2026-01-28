/*
  Copyright (C) 2025, 2026 CERN for the benefit of the ATLAS collaboration
*/

/// Unit tests for SQLiteDBSvc
#include "TestTools/random.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"
#include "SQLiteDBSvc/ISQLiteDBSvc.h"
#include "SQLiteDBSvc/Statement.h"
#include "TestTools/initGaudi.h"

// STL includes
#include <cassert>
#include <iostream>

int main() {
  std::cout << "Starting SQLiteDBSvc unit tests\n";

  ISvcLocator* svcLoc;
  if (!Athena_test::initGaudi("SQLiteDBSvc/SQLiteDBSvc_test.txt", svcLoc)) {
    return 1;
  }

  SmartIF<ISQLiteDBSvc> dbSvc =
      svcLoc->service<ISQLiteDBSvc>("SQLiteDBSvc/TestDBSvc", false);

  if (!dbSvc.isValid()) {
    std::cout << "Failed to get SQLiteDBSvc\n";
    return 1;
  }

  dbSvc
      ->createStatement(
          "CREATE TABLE test ("
          "col1 INTEGER,"
          "col2 REAL,"
          "col3 INTEGER,"
          "timestamp TEXT,"
          "id INTEGER PRIMARY KEY)")
      .run();
  std::cout << "Table created" << std::endl;

  SQLite::Statement insertStatement = dbSvc->createStatement(
      "INSERT INTO test (col1, col2, col3, timestamp) "
      "VALUES (?, ?, ?, datetime(\"now\"))");

  uint32_t seed = 12;

  for (int i = 1; i <= 250; ++i) {
    // Precompute random numbers because order of function
    // argument evaluation is undefined, and gcc and clang
    // do it in opposite orders.
    int randInt = Athena_test::randi_seed (seed, 1200);
    float randFloat = Athena_test::randf_seed (seed, 1200);
    insertStatement.run(i, randFloat, randInt);
  }

  SQLite::Statement selectStatement = dbSvc->createStatement(
      "SELECT id, timestamp, col2 FROM test WHERE id % ? == 0");
  auto res = selectStatement.run<int, std::string, float>(2);
  static_assert(std::is_same_v<decltype(res),
                               std::vector<std::tuple<int, std::string, float>>>);
  assert(res.size() == 125);
  std::cout << " First result: ";
  for (const auto& it : res) {
    std::cout << std::get<0>(it) << " -- " << std::get<2>(it) << ", ";
  }
  std::cout << std::endl;

  auto res2 = selectStatement.run<std::int64_t, std::string, double>(5);
  static_assert(
      std::is_same_v<decltype(res2),
                     std::vector<std::tuple<std::int64_t, std::string, double>>>);
  assert(res2.size() == 50);
  return 0;
}
