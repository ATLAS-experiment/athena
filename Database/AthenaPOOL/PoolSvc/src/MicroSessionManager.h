/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_MICROSESSIONMANAGER_H
#define POOLSVC_MICROSESSIONMANAGER_H

#include "GaudiKernel/IFileMgr.h"

#include <string>
#include <set>

class IStorageSvc;
class SessionTool;
class DatabaseHandler;

/** @class MicroSessionManager
 *
 * MicroSessionManager is a class taking care of starting
 * sessions for a given major technology and managing
 * the individual database connections.
 *
 */

class MicroSessionManager final {
public:
  /// Constructor
  MicroSessionManager( SessionTool& sessionTool, long technology, int ageLimit );
  /// Destructor
  ~MicroSessionManager();

  /// Connects to a database.
  DatabaseHandler* connect( Io::IoFlag transType,
                            const std::string& fid,
                            const std::string& pfn );

  /// Disconnects from a database.
  void disconnect( DatabaseHandler* database );

  /// Disconnects from all the databases
  bool disconnectAll();

  /// Fetches the FID by trying to temporatily connect to a database.
  std::string fidForPfn( const std::string& pfn );

  bool attributeOfType( const std::string& attributeName,
                        void* data,
                        const std::type_info& typeInfo,
                        const std::string& option );

  bool setAttributeOfType( const std::string& attributeName,
                           const void* data,
                           const std::type_info& typeInfo,
                           const std::string& option );

private:
  SessionTool&               m_sessionTool;
  IStorageSvc*               m_storageSvc;
  long                       m_technology;
  std::set<DatabaseHandler*> m_databaseHandlers;
};

#endif
