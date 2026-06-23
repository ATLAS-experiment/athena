/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SqliteRecord.h
 *
 * @brief Declaration of the SqliteRecord class
 *
 */

#ifndef RDBACCESSSVC_SQLITERECORD_H
#define RDBACCESSSVC_SQLITERECORD_H

#include "RDBAccessSvc/IRDBRecord.h"
#include <map>
#include <memory>
#include <variant>
#include <string>
#include <string_view>
#include <tuple>

enum SqliteInpType
{
  SQLITEINP_INT
  , SQLITEINP_LONG
  , SQLITEINP_FLOAT
  , SQLITEINP_DOUBLE
  , SQLITEINP_STRING
  , SQLITEINP_UNDEF
};

typedef std::variant<int
                     , long
                     , float
                     , double
                     , std::string> SqliteInp;

typedef std::map<std::string,SqliteInpType, std::less<>> SqliteInpDef;
typedef std::shared_ptr<SqliteInpDef> SqliteInpDef_ptr;

/**
 * @class SqliteRecord
 *
 * @brief SqliteRecord is one record in the SqliteRecordset object
 */

class SqliteRecord final : public IRDBRecord
{
 public:
  SqliteRecord(SqliteInpDef_ptr def);
  SqliteRecord() = delete;
  SqliteRecord (const SqliteRecord&) = delete;
  SqliteRecord& operator= (const SqliteRecord&) = delete;

  /// Destructor
  ~SqliteRecord() override;

  /// Check if the field value is NULL
  /// @param field [IN] field name
  /// @retun TRUE if the field is NULL, FALSE otherwise
  bool isFieldNull(std::string_view  field) const override;

  /// Get int field value
  /// @param field [IN] field name
  /// @return field value
  int getInt(std::string_view  field) const override;

  /// Get long field value
  /// @param field [IN] field name
  /// @return field value
  long getLong(std::string_view  field) const override;

  /// Get double field value
  /// @param field [IN] field name
  /// @return field value
  double getDouble(std::string_view  field) const override;

  /// Get float field value
  /// @param field [IN] field name
  /// @return field value
  float getFloat(std::string_view  field) const override;

  /// Get string field value
  /// @param field [IN] field name
  /// @return field value
  virtual const std::string&  getString(std::string_view  field) const override;

  // Access array values by index
  // arrays are implemented using the field with names like NAME_0, NAME_1 etc.

  /// Get array int field value
  /// @param field [IN] field name
  /// @param index [IN] index in the array
  /// @return field value
  int getInt(std::string_view  field, unsigned int index) const override;

  /// Get array long field value
  /// @param field [IN] field name
  /// @param index [IN] index in the array
  /// @return field value
  long getLong(std::string_view  field, unsigned int index) const override;

  /// Get array double field value
  /// @param field [IN] field name
  /// @param index [IN] index in the array
  /// @return field value
  double getDouble(std::string_view  field, unsigned int index) const override;

  /// Get array float field value
  /// @param field [IN] field name
  /// @param index [IN] index in the array
  /// @return field value
  float getFloat(std::string_view  field, unsigned int index) const override;

  /// Get array string field value
  /// @param field [IN] field name
  /// @param index [IN] index in the array
  /// @return field value
  virtual const std::string&  getString(std::string_view  field, unsigned int index) const override;

  /// Dump to cout
  void dump() const;

  void addValue(std::string_view  field, SqliteInp value);

 private:
  typedef std::map<std::string,SqliteInp, std::less<>> Record;
  typedef Record::const_iterator RecordCIterator;
  enum FieldCheckCode
  {
    FIELD_CHECK_OK
    , FIELD_CHECK_BAD_NAME
    , FIELD_CHECK_BAD_TYPE
    , FIELD_CHECK_NULL_VAL
  };
  typedef std::tuple<RecordCIterator,FieldCheckCode> FieldCheckResult;

  SqliteInpDef_ptr m_def;
  Record           m_record;

  FieldCheckResult checkField(std::string_view  field, SqliteInpType fieldType) const;
  void handleError(std::string_view  field, FieldCheckCode checkCode) const;
};

inline SqliteRecord::FieldCheckResult SqliteRecord::checkField(std::string_view  field
							       , SqliteInpType fieldType) const
{
  FieldCheckCode checkCode{FIELD_CHECK_OK};
  RecordCIterator checkIt{m_record.end()};

  auto defIt = m_def->find(field);
  if(defIt==m_def->end()) checkCode = FIELD_CHECK_BAD_NAME;
  else if(defIt->second!=fieldType) checkCode = FIELD_CHECK_BAD_TYPE;

  if(checkCode==FIELD_CHECK_OK) {
    checkIt = m_record.find(field);
    if(checkIt==m_record.end()) {
      checkCode = FIELD_CHECK_NULL_VAL;
    }
  }
  return std::make_tuple(checkIt,checkCode);
}

#endif

