/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RDBACCESSSVC_IRDBQUERY_H
#define RDBACCESSSVC_IRDBQUERY_H

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include <string>

class IRDBQuery
{
 public:
  IRDBQuery() = default;
  virtual ~IRDBQuery() = default;

  virtual void execute() = 0;
  virtual long size() = 0;
  virtual void finalize() = 0;
  virtual void setOrder(const std::string&) = 0;
  virtual void addToOutput(const std::string&) = 0;

  virtual bool next() = 0;

  template<typename T> const T& data(const std::string&);
  template<typename T> const T& data(unsigned int);
  bool isNull(const std::string&);
  bool isNull(unsigned int);

 protected:
  const coral::AttributeList* m_attrList{nullptr};
};

template<typename T> const T& IRDBQuery::data(const std::string& field)
{
  return (*m_attrList)[field].data<T>();
}

template<typename T> const T& IRDBQuery::data(unsigned int fieldInd)
{
  return (*m_attrList)[fieldInd].data<T>();
}

inline bool IRDBQuery::isNull(const std::string& field)
{
  return (*m_attrList)[field].isNull();
}

inline bool IRDBQuery::isNull(unsigned int fieldInd)
{
  return (*m_attrList)[fieldInd].isNull();
}

#endif
