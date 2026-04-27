/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SqliteRecord.cxx
 *
 * @brief Implementation of the SqliteRecord class
 *
 */


#include "SqliteRecord.h"
#include "boost/io/ios_state.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <format>

SqliteRecord::SqliteRecord(SqliteInpDef_ptr def)
  : m_def(std::move(def))
{
}

SqliteRecord::~SqliteRecord()
{
}

bool SqliteRecord::isFieldNull(std::string_view  field) const 
{
  if(m_record.find(field)!=m_record.end()) return false;
  if(m_def->find(field)==m_def->end()) throw std::runtime_error( std::format("Wrong name for the field {}",field));
  return true;
}

int SqliteRecord::getInt(std::string_view  field) const
{
  auto [recIt,checkCode] = checkField(field,SQLITEINP_INT);
  if(checkCode==FIELD_CHECK_OK) {
    return std::get<int>(recIt->second);
  }
  handleError(field,checkCode);
  return 0;
}

long SqliteRecord::getLong(std::string_view  field) const
{
  // Our database does not support LONG data types at this moment
  return (long)getInt(field);
}

double SqliteRecord::getDouble(std::string_view  field) const
{
  auto [recIt,checkCode] = checkField(field,SQLITEINP_DOUBLE);
  if(checkCode==FIELD_CHECK_OK) {
    return std::get<double>(recIt->second);
  }
  handleError(field,checkCode);
  return 0.;
}

float SqliteRecord::getFloat(std::string_view  field) const
{
  // SQLite stores REAL as double
  return (float)getDouble(field);
}

const std::string&  SqliteRecord::getString(std::string_view  field) const
{
  auto [recIt,checkCode] = checkField(field,SQLITEINP_STRING);
  if(checkCode==FIELD_CHECK_OK) {
    return std::get<std::string>(recIt->second);
  }
  handleError(field,checkCode);
  throw std::runtime_error("Unexpected error in SqliteRecord::getString()");
}

int SqliteRecord::getInt(std::string_view  field, unsigned int index) const
{
  return getInt(std::format("{}_{}", field, index));
}

long SqliteRecord::getLong(std::string_view  field, unsigned int index) const
{
  return getLong(std::format("{}_{}", field, index));
}

double SqliteRecord::getDouble(std::string_view  field, unsigned int index) const
{
  return getDouble(std::format("{}_{}", field, index));
}

float SqliteRecord::getFloat(std::string_view  field, unsigned int index) const
{
  return getFloat(std::format("{}_{}", field, index));
}

const std::string&  SqliteRecord::getString(std::string_view  field, unsigned int index) const
{
  return getString(std::format("{}_{}", field, index));
}

void SqliteRecord::addValue(std::string_view  field, SqliteInp value)
{
  auto [it,result] = m_record.emplace(std::string{field}, std::move(value));
  if(!result) throw std::runtime_error(std::format("Unexpected error when adding new value for the field {}. Duplicate field name?", field)); 
}

void SqliteRecord::dump() const
{
  boost::io::ios_all_saver saver (std::cout);
  bool first{true};
  for(const auto& [colName,colType] : *m_def) {
    if(first) {
      first = false;
    }
    else {
      std::cout << ", ";
    }
    auto recIt = m_record.find(colName);
    bool fieldNull = (recIt==m_record.end());
    std::cout << "[" << colName << " (";
    switch(colType) {
    case SQLITEINP_INT:
      std::cout << "int) : " << (fieldNull? "NULL" : std::to_string(std::get<int>(recIt->second))) << "]";
      break;
    case SQLITEINP_LONG:
      std::cout << "long) : " << (fieldNull? "NULL" : std::to_string(std::get<long>(recIt->second))) << "]";
      break;
    case SQLITEINP_FLOAT:
      std::cout << "float) : ";
      if (fieldNull) {
	      std::cout << "NULL";
      }
      else {
	      std::cout << std::setprecision(10) << std::get<float>(recIt->second) << "]";
      }
      break;
    case SQLITEINP_DOUBLE:
      std::cout << "double) : ";
      if (fieldNull) {
	      std::cout << "NULL";
      }
      else {
	      std::cout << std::setprecision(10) << std::get<double>(recIt->second) << "]";
      }
      break;
    case SQLITEINP_STRING:
      std::cout << "string) : " << (fieldNull? "NULL" : ("\"" + std::get<std::string>(recIt->second)) + "\"") << "]";
      break;
    default:
      std::cout << "ERROR) : ]";
    }
  }
  std::cout << std::endl;
}

void 
SqliteRecord::handleError(std::string_view  field, FieldCheckCode checkCode) const
{
  switch(checkCode) {
  case FIELD_CHECK_BAD_NAME:
    throw std::runtime_error( std::format("handleError: Wrong name for the field {}",field));
  case FIELD_CHECK_BAD_TYPE:
    throw std::runtime_error( std::format("handleError: Wrong data type requested for the field {}",field));
  case FIELD_CHECK_NULL_VAL:
    throw std::runtime_error( std::format("handleError: {} is NULL", field));
  default:
    break;
  }
  return;
}
