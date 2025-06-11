/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INCLUDE_PERSISTENCYSVC_USERSESSION_H
#define INCLUDE_PERSISTENCYSVC_USERSESSION_H

#include "PersistencySvc/ISession.h"

namespace pool {

  class DatabaseConnectionPolicy;

  namespace PersistencySvc {

    // forward declarations
    class DatabaseRegistry;
    class TechnologyDispatcher;
    class GlobalTransaction;

    /** @class UserSession
     *
     *  UserSession is an implementation of the ISession interface
     *
     */

    class UserSession : virtual public ISession
    {
    public:
      /// Constructor
      UserSession( IFileCatalog& fileCatalog );

      /// Destructor
      virtual ~UserSession();

      UserSession (const UserSession&) = delete;
      UserSession& operator= (const UserSession&) = delete;

      // Signatures needed for the PersistencySvc
      DatabaseRegistry& registry();
      TechnologyDispatcher& technologyDispatcher();

      /// Sets the default policy when databases are opened/connected
      virtual void setDefaultConnectionPolicy( const DatabaseConnectionPolicy& policy ) override final;

      /// Retrieves the default connection policy
      virtual const DatabaseConnectionPolicy& defaultConnectionPolicy() const override final;

      /** Explicitly disconnects all the databases.
       *  If a transaction is active, then all the changes since the last commit are aborted.
       */
      virtual bool disconnectAll() override final;

       /// Returns the transaction object
      virtual ITransaction& transaction() override final;
      virtual const ITransaction& transaction() const override final;

      /// Returns a vector with the file identifiers of the presently open databases.
      virtual std::vector< std::string > connectedDatabases() const override final;
      
      /// Creates and returns a new database handle object
      virtual std::unique_ptr<IDatabase>
      databaseHandle( const std::string& dbName, DatabaseSpecification::NameType dbNameType ) override final;

      /// Returns the file catalog in use
      virtual IFileCatalog& fileCatalog() override final;

      /// Set the file catalog to be used
      void setFileCatalog(IFileCatalog& catalog);

      /// Returns the object holding the technology specific attributes for a given technology domain
      virtual const ITechnologySpecificAttributes&
      technologySpecificAttributes( long technology ) const override final;

      virtual  ITechnologySpecificAttributes&
      technologySpecificAttributes( long technology ) override final;

      /// Returns the global transaction object
      ITransaction& globalTransaction();
      
    private:
      DatabaseConnectionPolicy*      m_policy;
      IFileCatalog*                  m_catalog;
      DatabaseRegistry*              m_registry;
      GlobalTransaction*             m_transaction;
      TechnologyDispatcher*          m_technologyDispatcher;
    };
  }
}

#endif
