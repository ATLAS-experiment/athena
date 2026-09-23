/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_MICROSESSIONMANAGER_H
#define POOLSVC_MICROSESSIONMANAGER_H

#include <string>
#include <set>

#include "StorageSvc/pool.h"

namespace pool {

  // forward declarations
  class IStorageSvc;

  // forward declarations
  class DatabaseRegistry;
  class DatabaseHandler;

  /** @class MicroSessionManager
   * 
   * MicroSessionManager is a class taking care of starting
   * sessions for a given major technology and managing
   * the individual database connections.
   *
   */

  class MicroSessionManager {
  public:
    /// Constructor
    MicroSessionManager( DatabaseRegistry& registry, long technology );
    /// Destructor
    virtual ~MicroSessionManager();

    /// Connects to the storage service
    bool connect( Io::IoFlag transType, int ageLimit );

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

    /// Return StorageSvc 
    IStorageSvc& getStorageSvc( ) { return *m_storageSvc; }

    virtual
    bool attributeOfType( const std::string& attributeName,
                          void* data,
                          const std::type_info& typeInfo,
                          const std::string& option );

    virtual
    bool setAttributeOfType( const std::string& attributeName,
                             const void* data,
                             const std::type_info& typeInfo,
                             const std::string& option );

  private:
    DatabaseRegistry&          m_registry;
    IStorageSvc*               m_storageSvc;
    bool                       m_inSession;
    long                       m_technology;
    std::set<DatabaseHandler*> m_databaseHandlers;
  };
}

#endif
