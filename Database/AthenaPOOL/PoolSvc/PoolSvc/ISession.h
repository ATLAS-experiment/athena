/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_ISESSION_H
#define POOLSVC_ISESSION_H

#include "PoolSvc/IDatabase.h"
#include "PoolSvc/ITransaction.h"

#include "DataModelRoot/RootType.h"

#include <string>
#include <memory>

class Placement;
class Token;

namespace pool {

  // forward declarations
  class ITechnologySpecificAttributes;
  class IFileCatalog;
  class IStorageSvc;

  /// Factory method to create a session object
  class ISession;
  std::unique_ptr<ISession> createSession( IFileCatalog& catalog, int ageLimit = -1 );

  /** @class ISession ISession.h PoolSvc/ISession.h
  *
  *  ISession is the interface class for user (macroscopic) sessions
  *  Every transaction and connection to a database and object reading
  *  and writing must be performed within a session. 
  *  It also provides access to the file catalog and to the technology specific attributes.
  */

  class ISession : public ITransaction {
  public:
    /// Explicitly disconnects all the databases.
    /// If a transaction is active, then all the changes since the last commit are aborted.
    virtual bool disconnectAll() = 0;

    /// Returns the transaction interface
    virtual ITransaction& transaction() = 0;
    virtual const ITransaction& transaction() const = 0;

    /// Returns a pointer to a database object. The user acquires ownership of that object.
    virtual std::unique_ptr<IDatabase> databaseHandle( const std::string& dbName,
                                                       DatabaseSpecification::NameType dbNameType ) = 0;

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

    /// Returns the file catalog in use
    virtual IFileCatalog& fileCatalog() = 0;

    /// Returns the object holding the technology specific attributes for a given technology domain
    virtual ITechnologySpecificAttributes& technologySpecificAttributes( long technology ) = 0;

    /// Return StorageSvc for a given technology used in this session
    virtual IStorageSvc& getStorageSvc( long technology ) = 0;

    /// virtual destructor for the interface
    virtual ~ISession() = default;
  };
}
#endif
