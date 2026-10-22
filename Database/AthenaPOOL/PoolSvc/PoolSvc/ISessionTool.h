/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ISESSIONTOOL_H
#define POOLSVC_ISESSIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "PoolSvc/IDatabase.h"
#include "DataModelRoot/RootType.h"

#include <string>
#include <memory>

namespace Gaudi {
  class IFileCatalog;
}
class Placement;
class Token;
namespace pool {
  class MicroSessionManager;
  class IStorageSvc;
}

  /** @class ISession ISession.h PoolSvc/ISession.h
  *
  *  ISession is the interface class for user (macroscopic) sessions
  *  Every connection to a database and object reading and writing must be performed within a session.
  *  It also provides access to the file catalog and to the technology specific attributes.
  */

class ISessionTool : virtual public AthAlgTool 
{
    public:

    /// Use this FileCatalog and DB ageing value for this session    
    virtual void setupFC( Gaudi::IFileCatalog& fileCatalog, int ageLimit = -1 ) = 0;
      
    /// Explicitly disconnects all the databases.
    /// If a transaction is active, then all the changes since the last commit are aborted.
    virtual bool disconnectAll() = 0;

    /// Starts a new transaction. Returns the success of the operation
    virtual bool start( Io::IoFlag type = Io::READ ) = 0;

    /// Commits the transaction.
    virtual bool commit() = 0;

    /// Commits the holds transaction.
    virtual bool commitAndHold() = 0;

    /// Returns the transaction type
    virtual Io::IoFlag openMode() const = 0;

    /// Returns a pointer to a database object. The user acquires ownership of that object.
    virtual std::unique_ptr<pool::IDatabase> 
    databaseHandle( const std::string& dbName, pool::DatabaseSpecification::NameType dbNameType ) = 0;

    /** Retrieves an object from persistent store and return with type information
    *  The handle to the reflection class is necessary to later delete the object.
    *  The Guid of the transient class is assumed to be the classID of the token
    *
    * @param  token   [IN]  reference to the token for the object
    * @param  object  [IN]  pointer to memory for the object (created if 0)
    *
    * @return void*   The data
    *
    * In case of failure zero is returned.
    */
    virtual void* readObject( const Token& token, void* object = 0 ) = 0;

    /**  registerForWrite registers an object for writing to the persistent medium
    *   higher level interactions with the framework are necessary.
    *
    * @param  place        [IN]  the placement hint
    * @param  object       [IN]  pointer to transient object which will be written
    * @param  type         [IN]  reflection class description with the layout of transient object
    *
    * @return Token*   the token address of the persistent object. I case of failure 0 is returned.
    */
    virtual Token* registerForWrite( const Placement& place, const void* object, const RootType& type ) = 0;

    /// Returns the technology given a technology type.
    virtual pool::MicroSessionManager& microSessionManager( long technology ) = 0;

    /// Return StorageSvc for a given technology used in this session
    virtual pool::IStorageSvc& getStorageSvc( long technology ) = 0;

    virtual void closeSession() = 0;

    virtual ~ISessionTool() = default;
};

#endif
