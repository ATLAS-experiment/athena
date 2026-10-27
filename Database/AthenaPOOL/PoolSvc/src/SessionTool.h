/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_SESSIONTOOL_H
#define POOLSVC_SESSIONTOOL_H

#include "PoolSvc/ISessionTool.h"

#include <map>

namespace pool {
  class DatabaseRegistry;
}

/** @class SessionTool
  *   *
  */
class SessionTool : public ISessionTool
{
  public:
    /// Constructors 
    explicit SessionTool(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);
 
   

    /// Destructor
    virtual ~SessionTool();

    SessionTool (const SessionTool&) = delete;
    SessionTool& operator= (const SessionTool&) = delete;

    virtual void setupFC( Gaudi::IFileCatalog& fileCatalog, int ageLimit ) override final;

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
    virtual void* readObject( const Token& token, void* object = 0 ) override final;


    /**  registerForWrite registers an object for writing to the persistent medium
     *   higher level interactions with the framework are necessary.
     *
     * @param  place        [IN]  the placement hint
     * @param  object       [IN]  pointer to transient object which will be written
     * @param  type         [IN]  reflection class description with the layout of transient object
     *
     * @return Token*   the token address of the persistent object. I case of failure 0 is returned.
     */
    virtual Token* registerForWrite( const Placement& place, const void* object, const RootType& type ) override final;


    // Signatures needed for the PersistencySvc
    pool::DatabaseRegistry& registry() {return *m_registry; }

    /** Explicitly disconnects all the databases.
     *  If a transaction is active, then all the changes since the last commit are aborted.
     */
    virtual bool disconnectAll() override final;

    /// Starts a new transaction. Returns the success of the operation
    virtual bool startTransaction( Io::IoFlag type = Io::READ ) override final;

    /// Commits the transaction.
    virtual bool commit() override final;

    /// Commits and holds the transaction.
    virtual bool commitAndHold() override final;

    /// Returns the transaction type
    virtual Io::IoFlag openMode() const override final { return m_transactionType; }

    /// Creates and returns a new database handle object
    virtual std::unique_ptr<pool::IDatabase>
    databaseHandle( const std::string& dbName, pool::DatabaseSpecification::NameType dbNameType ) override final;

    /// Returns the file catalog in use
    virtual Gaudi::IFileCatalog& fileCatalog();

    /// Return StorageSvc for a given technology used in this session
    virtual pool::IStorageSvc& getStorageSvc( long technology ) override final;

    /// Returns the technology given a technology type.
    pool::MicroSessionManager& microSessionManager( long technology ) override final;

    void closeSession() override final;
    StatusCode finalize() override final;

  private:
    Gaudi::IFileCatalog*           m_catalog;
    int                            m_ageLimit;
    std::unique_ptr<pool::DatabaseRegistry>  m_registry;
    Io::IoFlag                     m_transactionType;
    std::map< long, std::unique_ptr<pool::MicroSessionManager> >    m_technologies;
  };


#endif
