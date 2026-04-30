/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INCLUDE_PERSISTENCYSVC_MICROSESSIONMANAGER_H
#define INCLUDE_PERSISTENCYSVC_MICROSESSIONMANAGER_H

#include <string>
#include <set>

#include "PersistencySvc/ITechnologySpecificAttributes.h"
#include "PersistencySvc/ITransaction.h"

namespace pool {

  // forward declarations
  class IStorageSvc;

  namespace PersistencySvc {

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

    class MicroSessionManager : virtual public ITechnologySpecificAttributes {
    public:
      /// Constructor
      MicroSessionManager( DatabaseRegistry& registry, long technology );
      /// Destructor
      virtual ~MicroSessionManager();

      /// Connects to the storage service
      bool connect( ITransaction::Type transType );

      /// Connects to a database.
      DatabaseHandler* connect( ITransaction::Type transType,
                                const std::string& fid,
                                const std::string& pfn,
                                long accessMode );

      /// Disconnects from a database.
      void disconnect( DatabaseHandler* database );

      /// Disconnects from all the databases
      bool disconnectAll();

      /// Returns the technology
      long technology() const;

      /// Fetches the FID by trying to temporatily connect to a database.
      std::string fidForPfn( const std::string& pfn );

    protected:
      virtual
      bool attributeOfType( const std::string& attributeName,
                            void* data,
                            const std::type_info& typeInfo,
                            const std::string& option ) override;

      virtual
      bool setAttributeOfType( const std::string& attributeName,
                               const void* data,
                               const std::type_info& typeInfo,
                               const std::string& option ) override;

    private:
      DatabaseRegistry&          m_registry;
      IStorageSvc*               m_storageSvc;
      bool                       m_inSession;
      long                       m_technology;
      std::set<DatabaseHandler*> m_databaseHandlers;
    };
  }
}

#endif
