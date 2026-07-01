/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SqliteReadSvc.h
 *
 * @brief Declaration of SqliteReadSvc class
 *
 */

#ifndef RDBACCESSSVC_SQLITEREADSVC_H
#define RDBACCESSSVC_SQLITEREADSVC_H

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "SqliteRecordset.h"

#include "AthenaBaseComps/AthService.h"

#include <sqlite3.h>
#include <string>
#include <map>
#include <mutex>

class ISvcLocator;

template <class TYPE> class SvcFactory;

// Map of recordset pointers by table name
typedef std::map<std::string, IRDBRecordset_ptr, std::less<>> RecordsetPtrMap;

/**
 * @class SqliteReadSvc
 *
 * @brief SqliteReadSvc implementats IRDBAccessSvc interface for reading
 *        plain tables in the Geometry SQLite database
 *
 */

class SqliteReadSvc final : public extends<AthService, IRDBAccessSvc>
{
 public:
  /// Standard Service Constructor
  SqliteReadSvc(const std::string& name, ISvcLocator* svc);

  StatusCode finalize() override;

  /// Open the SQLite database
  /// This method has no effect if the connection has already been opened
  /// @param connName [IN] path to the SQLite database file
  /// @return success/failure
  bool connect(std::string_view connName) override;

  /// Dummy overrider of the virtual function
  /// @return success/failure
  bool disconnect(std::string_view connName) override;

  /// Closes the database connection
  /// @return success/failure
  bool shutdown(std::string_view connName) override;

  /// Provides access to the Recordset object containing HVS-tagged data.
  /// @param node [IN] name of the table. Other input parameters are dummy
  /// @return pointer to the recordset object
  IRDBRecordset_ptr getRecordsetPtr(std::string_view node
				    , std::string_view tag
				    , std::string_view tag2node 
				    , std::string_view connName) override;

  /// Dummy overrider of the virtual function
  /// @param childNode [IN] the name of the table
  /// @return the name of the table if exists, otherwise an empty string
  std::string getChildTag(const std::string& childNode
			  , const std::string& 
			  , const std::string& 
			  , const std::string& ) override;

  /// Dummy overrider of the virtual function (for now)
  std::unique_ptr<IRDBQuery> getQuery(const std::string& node
				      , const std::string& 
				      , const std::string& 
				      , const std::string& ) override;

  /// Dummy overrider of the virtual function
  void getTagDetails(RDBTagDetails& tagDetails
		     , const std::string& tag
		     , const std::string& ) override;

private:
  RecordsetPtrMap m_recordsets;
  sqlite3*        m_db{nullptr};
  std::mutex      m_sessionMutex;
};

#endif 
