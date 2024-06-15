/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IRDBRecordset.h
 *
 * @brief Definition of the abstract IRDBRecordset interface 
 *
 * @author Vakho Tsulaia <Vakhtang.Tsulaia@cern.ch>
 *
 * $Id: IRDBRecordset.h,v 1.7 2008-12-16 14:05:40 dquarrie Exp $
 */

#ifndef RDBACCESSSVC_IRDBRECORDSET_H
#define RDBACCESSSVC_IRDBRECORDSET_H

#include <string>
#include <vector>
#include <memory>

#include "RDBAccessSvc/IRDBRecord.h"
using IRDBRecord_ptr = std::unique_ptr<IRDBRecord>;
using RecordsVector = std::vector<IRDBRecord_ptr>;

/**
 * @class IRDBRecordset
 *
 * @brief IRDBRecordset is an abstract interface to the RDB Recordsets,
 * which represent a snapshot of HVS-tagged data tables in the
 * relational database
 */

class IRDBRecordset
{
 public:
  virtual ~IRDBRecordset() = default;

  /// @return number of records
  virtual unsigned int size() const = 0;

  /// @return node name
  virtual std::string nodeName() const = 0;

  /// @return tag name
  virtual std::string tagName() const = 0;

  /// @param index [IN] index of the record
  /// @return RDBRecord by index
  virtual const IRDBRecord* operator[](unsigned int index) const= 0;

  typedef RecordsVector::const_iterator const_iterator;

  /// @return begin iterator
  virtual const_iterator begin() const=0;

  /// @return end iterator
  virtual const_iterator end() const=0;

};

#endif
