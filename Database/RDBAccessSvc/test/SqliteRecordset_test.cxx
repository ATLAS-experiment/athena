/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IOVDbSvc/test/SqliteRecordset_test.cxx
 * @author Shaun Roe
 * @date Jun, 2025
 * @brief Some tests for SqliteRecordset 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE RDBAccessSvc


#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include "src/SqliteRecordset.h"
#include <stdexcept>
//
#include <iostream>
#include <sqlite3.h>
#include <filesystem>  // Required for file deletion
#include "CxxUtils/checker_macros.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

struct cout_redirect {
    cout_redirect( std::streambuf * new_buffer ) 
        : old( std::cout.rdbuf( new_buffer ) )
    { }

    ~cout_redirect( ) {
        std::cout.rdbuf( old );
    }
    std::streambuf * old;
};


class DummySqlite {
public:
  explicit DummySqlite(const std::string& filename = "example.db")
    : m_dbFilename(filename), m_db(nullptr){
    // Open or create the SQLite database
    int exit = sqlite3_open(m_dbFilename.c_str(), &m_db);
    if (exit != SQLITE_OK) {
      std::cerr << "Error opening/creating database: " << sqlite3_errmsg(m_db) << std::endl;
      m_db = nullptr;
      throw std::runtime_error("Failed to open database");
    }
    std::cout << "Database '" << m_dbFilename << "' opened/created successfully.\n";
    // SQL to create the table
    const char* createTableSQL = R"sql(
      CREATE TABLE IF NOT EXISTS tbl1 (
          column1 TEXT,
          column2 INT
      );
    )sql";
    char* errorMessage = nullptr;
    exit = sqlite3_exec(m_db, createTableSQL, nullptr, nullptr, &errorMessage);
    if (exit != SQLITE_OK) {
      std::cerr << "Error creating table: " << errorMessage << std::endl;
      sqlite3_free(errorMessage);
      sqlite3_close(m_db);
      m_db = nullptr;
      throw std::runtime_error("Failed to create table");
    }
    std::cout << "Table 'tbl1' created successfully.\n";
  }
  sqlite3 * ptr() const { return m_db;}

  ~DummySqlite() {
    if (m_db) {
      sqlite3_close(m_db);
      std::cout << "Database closed.\n";
    }
    try {
      if (std::filesystem::exists(m_dbFilename)) {
          std::filesystem::remove(m_dbFilename);
          std::cout << "Database file '" << m_dbFilename << "' deleted.\n";
      }
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Failed to delete database file: " << e.what() << std::endl;
    }
  }

private:
  std::string m_dbFilename;
  sqlite3* m_db;
};


BOOST_AUTO_TEST_SUITE(SqliteRecordsetTest)
  BOOST_AUTO_TEST_CASE(canDefaultInstantiate){
    BOOST_CHECK_NO_THROW(SqliteRecordset s);
  }
  
  BOOST_AUTO_TEST_CASE(emptyRecordsetHasExpectedProperties){
    SqliteRecordset s;
    BOOST_CHECK_EQUAL(s.size(), 0 );
    BOOST_CHECK_EQUAL(s.nodeName(), "");
    BOOST_CHECK_EQUAL(s.tagName(), "");
    //
    BOOST_CHECK((s.begin() == s.end()));
    BOOST_CHECK_THROW(s.getData(nullptr, "nodename"), std::runtime_error);
  }
  BOOST_AUTO_TEST_CASE(getDataThrowsForEmptyNodeName){
    SqliteRecordset s;
    DummySqlite db("dummyDb");
    BOOST_CHECK_THROW(s.getData(db.ptr(), ""), std::runtime_error);
  }
  BOOST_AUTO_TEST_CASE(infoMessageGeneratedForNonexistantNodeName){
    SqliteRecordset s;
    DummySqlite db("dummyDb");
    boost::test_tools::output_test_stream output;
    // accessing a non-existing table does not throw, but gives INFO level message
    const std::string infoMsg("INFO porky table is not found in the database\n");
    //capture 'cout' inside this scope
    {
      cout_redirect guard( output.rdbuf( ) );
      BOOST_CHECK_NO_THROW(s.getData(db.ptr(), "porky"));
    }
    BOOST_CHECK( output.str().find(infoMsg) != std::string::npos);
  }
  
  BOOST_AUTO_TEST_CASE(dbNotInCorrectFormat){
    SqliteRecordset s;
    DummySqlite db("dummyDb");
    boost::test_tools::output_test_stream output;
    // accessing an existing table in db with incorrect format 
    // (ancillary table doesn't exist) generates an error message
    const std::string errMsg("SQLite Error: no such column: tbl1_data_id");
    //capture 'cout' inside this scope
    {
      cout_redirect guard( output.rdbuf( ) );
      BOOST_CHECK_NO_THROW(s.getData(db.ptr(), "tbl1"));
    }
    BOOST_CHECK(output.str().find(errMsg) != std::string::npos);
    BOOST_CHECK_EQUAL(s.size(), 0 );
  }
  
  
BOOST_AUTO_TEST_SUITE_END()
