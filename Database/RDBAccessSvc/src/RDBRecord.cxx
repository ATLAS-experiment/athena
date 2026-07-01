/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file RDBRecord.cxx
 *
 * @brief Implementation of RDBRecord class
 *
 * @author Vakho Tsulaia <Vakhtang.Tsulaia@cern.ch>
 *
 * $Id: RDBRecord.cxx,v 1.11 2006-10-23 15:24:06 tsulaia Exp $
 */


#include "RDBRecord.h"
#include "RelationalAccess/ICursor.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"

#include <stdexcept>
#include <format>

RDBRecord::RDBRecord(const coral::AttributeList& attList
		     , const std::string& tableName)
  : m_values(0)
  , m_tableName(tableName)
{
  // Copy attList.  Try to avoid sharing, for thread-safety.
  m_values = new coral::AttributeList(attList.specification(), false);
  m_values->fastCopyData (attList);

  for(unsigned int i=0; i<m_values->size(); i++) {
    std::string key = (*m_values)[i].specification().name();
    m_name2Index[key] = i;
  }
}

RDBRecord::~RDBRecord()
{
  delete m_values;
}

template<typename T>
const T& RDBRecord::getGeneric(std::string_view fieldName) const
{
  std::string name =std::format("{}.{}", m_tableName, fieldName);
  FieldName2ListIndex::const_iterator it = m_name2Index.find(name);
  if(it==m_name2Index.end()) {
    throw std::runtime_error(std::format("Wrong name for the field {}", name));  
  }

  const coral::AttributeList& values = *m_values;
  const auto &itr = values[it->second];
  if(itr.specification().type()==typeid(T)) {
    return itr.data<T>();
  } else {
    throw std::runtime_error(std::format("Field {} is NOT a type of {}", fieldName, typeid(T).name()));  
  }
}

RDBRecord::FieldName2ListIndex::const_iterator RDBRecord::getItr(std::string_view fieldName) const
{
  const std::string name = std::format("{}.{}", m_tableName, fieldName);
  FieldName2ListIndex::const_iterator it = m_name2Index.find(name);
  if(it==m_name2Index.end()) {
    throw std::runtime_error( "Wrong name for the field " + name);
  }
  return it;
}

RDBRecord::FieldName2ListIndex::const_iterator RDBRecord::getItr(std::string_view fieldName, int index) const
{
  const std::string name =std::format("{}.{}_{}", m_tableName, fieldName, index);
  FieldName2ListIndex::const_iterator it = m_name2Index.find(name);
  if(it==m_name2Index.end()) {
    throw std::runtime_error(std::format("Wrong name for the array field {}.{} or index={} is out of range.",m_tableName,fieldName,index));
  }
  return it;
}


bool RDBRecord::isFieldNull(std::string_view fieldName) const 
{
  auto it = getItr(fieldName);

  const coral::AttributeList& values = *m_values;
  return values[it->second].isNull();
}

int RDBRecord::getInt(std::string_view fieldName) const
{
  auto it = getItr(fieldName);
  const coral::AttributeList& values = *m_values;
  const auto& item = values[it->second];
  if(item.specification().type()==typeid(int)) {
    return item.data<int>();
  }
  else if(item.specification().type()==typeid(long)) {
    return (int)item.data<long>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of integer type", fieldName));
  }
}

long RDBRecord::getLong(std::string_view fieldName) const
{
  auto it = getItr(fieldName);
  const coral::AttributeList& values = *m_values;
  const auto& item = values[it->second];
  if(item.specification().type()==typeid(long)) {
    return item.data<long>();
  }
  else if(item.specification().type()==typeid(int)) {
    return (long)item.data<int>();
  }
  else if(item.specification().type()==typeid(long long)) {
    return (long)item.data<long long>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of long type",fieldName));
  }
}

double RDBRecord::getDouble(std::string_view fieldName) const
{
  return getGeneric<double>(fieldName);
}

float RDBRecord::getFloat(std::string_view fieldName) const
{
  return getGeneric<float>(fieldName);
}

const std::string& RDBRecord::getString(std::string_view fieldName) const
{
  return getGeneric<std::string>(fieldName);
}

int RDBRecord::getInt(std::string_view fieldName, unsigned int index) const
{
  auto it = getItr(fieldName, index);
  const coral::AttributeList& values = *m_values;
  const auto &item = values[it->second];
  if(item.specification().type()==typeid(int)) {
    return item.data<int>();
  }
  else if(item.specification().type()==typeid(long)) {
    return (int)item.data<long>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of integer type", fieldName));
  }
}

long RDBRecord::getLong(std::string_view fieldName, unsigned int index) const
{
  auto it = getItr(fieldName, index);
  const coral::AttributeList& values = *m_values;
  const auto &item = values[it->second];
  if(item.specification().type()==typeid(long)) {
    return item.data<long>();
  }
  else if(item.specification().type()==typeid(int)) {
    return (long)item.data<int>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of long type", fieldName));
  }
}

double RDBRecord::getDouble(std::string_view fieldName, unsigned int index) const
{
  auto it = getItr(fieldName, index);
  const coral::AttributeList& values = *m_values;
  const auto &item = values[it->second];
  if(item.specification().type()==typeid(double)) {
    return item.data<double>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of double type",fieldName));
  }
}

float RDBRecord::getFloat(std::string_view fieldName, unsigned int index) const
{
  auto it = getItr(fieldName, index);
  const coral::AttributeList& values = *m_values;
  const auto &item = values[it->second];
  if(item.specification().type()==typeid(float)) {
    return item.data<float>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of float type", fieldName));
  }
}

const std::string& RDBRecord::getString(std::string_view fieldName, unsigned int index) const
{
  auto it = getItr(fieldName, index);
  const coral::AttributeList& values = *m_values;
  const auto &item = values[it->second];
  if(item.specification().type()==typeid(std::string)) {
    return item.data<std::string>();
  }
  else {
    throw std::runtime_error( std::format("Field {} is NOT of string type", fieldName));
  }
}

bool RDBRecord::operator!=(const RDBRecord& rhs) const
{
  const coral::AttributeList& myAttList = *m_values;
  const coral::AttributeList& rhsAttList = *rhs.m_values;

  if(myAttList.size()!=rhsAttList.size()) return true;

  for(size_t i(0); i<myAttList.size(); ++i) {
    const coral::Attribute& myAtt = myAttList[i];
    const std::string name = myAtt.specification().name();
    bool exists(false);
    for(size_t j(0); j<rhsAttList.size(); ++j) {
      const coral::Attribute& rhsAtt = rhsAttList[j];
      if(rhsAtt.specification().name()==name) {
	if(myAtt!=rhsAtt) {
	  return true;
	}
	exists = true;
	break;
      }
    }// Go through the attributes in the RHS list
    if(!exists)
      return true;
  }
  return false;
}

std::ostream& RDBRecord::toOutputStream(std::ostream& os) const
{
  m_values->toOutputStream(os);
  return os;
}
