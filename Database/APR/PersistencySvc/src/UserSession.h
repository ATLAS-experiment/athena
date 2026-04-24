/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INCLUDE_PERSISTENCYSVC_USERSESSION_H
#define INCLUDE_PERSISTENCYSVC_USERSESSION_H

#include "AthenaBaseComps/AthMessaging.h"
#include "PersistencySvc/ISession.h"
#include "StorageSvc/DbPrint.h"

#include <map>

namespace pool {

  class DatabaseConnectionPolicy;

  // forward declarations
  class DatabaseRegistry;
  class MicroSessionManager;

  /** @class UserSession
   *
   *  UserSession is an implementation of the ISession interface
   *
   */

  class UserSession : public ISession, public APRMessaging
  {
  public:
    /// Constructor
    explicit UserSession( IFileCatalog& fileCatalog );

    /// Destructor
    virtual ~UserSession();

    UserSession (const UserSession&) = delete;
    UserSession& operator= (const UserSession&) = delete;


    /** Retrieves an object from persistent store and return with type information
     *  The handle to the reflection class is necessary to later delete the object.
     *  The Guid of the transient class is assumed to be the classID of the token
     *
     * @param  token   [IN]  reference to the token for the object
     * @param  object  [IN]  pointer to memory for the object (created if 0)
     *
     * @return void*   The data.
     *
     * In case of failure zero is returned.
     */
    virtual void* readObject( const Token& token, void* object = 0 ) override;


    /**  registerForWrite registers an object for writing to the persistent medium
     *   higher level interactions with the framework are necessary.
     *
     * @param  place        [IN]  the placement hint
     * @param  object       [IN]  pointer to transient object which will be written
     * @param  type         [IN]  reflection class description with the layout of transient object
     *
     * @return Token*   the token address of the persistent object. I case of failure 0 is returned.
     */
    virtual Token* registerForWrite( const Placement& place, const void* object, const RootType& type ) override;


    // Signatures needed for the PersistencySvc
    DatabaseRegistry& registry();
    //TechnologyDispatcher& technologyDispatcher();

    /// Sets the default policy when databases are opened/connected
    virtual void setDefaultConnectionPolicy( const DatabaseConnectionPolicy& policy ) override final;

    /// Retrieves the default connection policy
    virtual const DatabaseConnectionPolicy& defaultConnectionPolicy() const override final;

    /** Explicitly disconnects all the databases.
     *  If a transaction is active, then all the changes since the last commit are aborted.
     */
    virtual bool disconnectAll() override final;

    /// Returns the transaction interface
    virtual ITransaction& transaction() override final { return *this; }
    virtual const ITransaction& transaction() const override final { return *this; }

    /// Starts a new transaction. Returns the success of the operation
    virtual bool start( ITransaction::Type type = READ ) override final;

    /// Commits the transaction.
    virtual bool commit() override final;

    /// Commits and holds the transaction.
    virtual bool commitAndHold() override final;

    /// Checks if the transaction is active
    virtual bool isActive() const override final { return m_transactionType != ITransaction::INACTIVE; }

    /// Returns the transaction type
    virtual ITransaction::Type type() const override final { return m_transactionType; }

    /// Returns the transaction type
    ITransaction::Type transactionType() const { return transaction().type(); }

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

    /// Returns the technology given a technology type.
    MicroSessionManager& microSessionManager( long technology );

  private:
    DatabaseConnectionPolicy*      m_policy;
    IFileCatalog*                  m_catalog;
    DatabaseRegistry*              m_registry;
    ITransaction::Type             m_transactionType;
    std::map< long, std::unique_ptr<MicroSessionManager> >    m_technologies;

  };
}

#endif
