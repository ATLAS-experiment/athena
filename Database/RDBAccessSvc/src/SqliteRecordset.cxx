/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SqliteRecordset.cxx
 *
 * @brief Implementation of the SqliteRecordset class
 *
 */

#include "SqliteRecordset.h"

#include <sqlite3.h>
#include <stdexcept>
#include <sstream>
#include <set>

SqliteRecordset::SqliteRecordset()
  : AthMessaging("SqliteRecordset")
  , m_def(std::make_shared<SqliteInpDef>())
{
}

void SqliteRecordset::getData(sqlite3* db, const std::string& nodeName)
{ 
  //the following should already be checked by the calling code and should never happen
  if (db == nullptr){
    throw std::runtime_error("SqliteRecordset::getData : db pointer is null");
  }
  if (nodeName.empty()){
    throw std::runtime_error("SqliteRecordset::getData : nodeName is empty");
  }
  ATH_MSG_DEBUG("getData for " << nodeName);
  m_nodeName = nodeName;

  std::ostringstream sql;

  // First check if the table exists in the database
  sql << "select * from " << m_nodeName;
  sqlite3_stmt* stTable{nullptr};
  int rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stTable, NULL);
  if(rc!=SQLITE_OK) {
    ATH_MSG_INFO(m_nodeName << " table is not found in the database");
    return;
  }

  sql << " order by " << m_nodeName << "_data_id";
  sqlite3_stmt* st{nullptr};
  rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &st, NULL);
  if(rc!=SQLITE_OK) {
    ATH_MSG_ERROR("Error occurred when preparing to fetch data for " << m_nodeName);
    ATH_MSG_ERROR("SQLite Error: " << sqlite3_errmsg(db));
    return;
  }
  int ctotal = sqlite3_column_count(st);

  bool all_ok{true};

  while(true) {
    rc = sqlite3_step(st);

    if(rc == SQLITE_ROW) {
      SqliteRecord* rec = new SqliteRecord(m_def);
      IRDBRecord_ptr record{rec};

      // Loop throug the fields of the retrieved record
      for(int i=0; i<ctotal; ++i) {

	// The feature of SQLite: if in the given record some fields have NULL values,
	// then the data type of the corresponding columns is reported as NULL.
	// This means that we need to be able to build the Def gradually, as we read
	// in new records of the table
	
	// Do we need to extend Def?
	std::string columnName = sqlite3_column_name(st,i);
	bool extendDef = (m_def->find(columnName)==m_def->end());

	auto columnType = sqlite3_column_type(st,i);
	SqliteInpType inpType{SQLITEINP_UNDEF};
	SqliteInp val;

	switch(columnType) {
	case SQLITE_INTEGER:
	  inpType = SQLITEINP_INT;
	  val = sqlite3_column_int(st,i);
	  break;
	case SQLITE_FLOAT:
	  inpType = SQLITEINP_DOUBLE;
	  val = sqlite3_column_double(st,i);
	  break;
	case SQLITE_TEXT:
	  inpType = SQLITEINP_STRING;
	  val = std::string((char*)(sqlite3_column_text(st,i)));
	  break;
	case SQLITE_BLOB:
	  inpType = SQLITEINP_STRING;
	  val = std::string((char*)(sqlite3_column_blob(st,i)));
	  break;
	case SQLITE_NULL:
	  continue;
	default:
	  break;
	}

	if(inpType==SQLITEINP_UNDEF) {
	  all_ok = false;
	  ATH_MSG_ERROR("Unexpected data type in column " << columnName << " of the table " << m_nodeName);
	  break;
	}

	if(extendDef) {
	  (*m_def)[columnName] = inpType;
	}
	rec->addValue(columnName,val);
      }
      m_records.push_back(std::move(record));
    }
    else if(rc == SQLITE_DONE) {
      break;
    }
    else {
      ATH_MSG_ERROR("Error occurred when fetching data for " << m_nodeName);
      ATH_MSG_ERROR("SQLite Error: " << sqlite3_errmsg(db));
      all_ok = false;
      break;
    }
  }

  if(!all_ok) {
    // Do memory cleanup
    m_records.clear();
  }
}

unsigned int SqliteRecordset::size() const 
{
  return m_records.size();
}

std::string SqliteRecordset::nodeName() const
{
  return m_nodeName;
}

const IRDBRecord* SqliteRecordset::operator[](unsigned int index) const 
{
  return m_records[index].get();
}
IRDBRecordset::const_iterator SqliteRecordset::begin() const
{
  return m_records.begin();
}

IRDBRecordset::const_iterator SqliteRecordset::end() const
{
  return m_records.end();
}
