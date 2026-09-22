/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_USERDATABASE_H
#define POOLSVC_USERDATABASE_H

#include "PoolSvc/IDatabase.h"
#include "PoolSvc/ISession.h"
#include "StorageSvc/DbPrint.h"
#include "StorageSvc/pool.h"

namespace pool {
  // forward declarations
  class IFileCatalog;
   
  // forward declarations
  class UserSession;
  class DatabaseHandler;
  class DatabaseRegistry;

  /** @class UserDatabase
   *
   *  UserDatabase is an implementation of the IDatabase interface.
   *
   */
  class UserDatabase : virtual public IDatabase,
                       public APRMessaging
  {
  public:
    /// Constructor
    UserDatabase( UserSession& session,
                  const std::string& name,
                  const DatabaseSpecification::NameType nameType );

    /// Destructor
    virtual ~UserDatabase();

    /// Connects explicitly to the database for read operations
    virtual void connectForRead() override;

    /// Reads an object given a token
    void* readObject( const Token& token, void* object = 0 );

    /// Connects explicitly to the database for write/update operations
    virtual void connectForWrite() override;

    /// Writes an object and returns a token
    Token* writeObject( const std::string& containerName,
                        long minorTechnology,
                        const void* object,
                        const RootType& type );

    /// Disconnects from the database
    virtual void disconnect() override;

    /// Returns the opening mode. It can be used to check whether the database is connected.
    virtual Io::IoFlag openMode() const override;

    /// Returns the file identifier of this database
    virtual const std::string& fid() override;

    /// Returns the physical file name of this database
    virtual const std::string& pfn() override;

    /** Sets the technology identifier for this database.
     *  This can only be called for newly created databases
     *  before the connect method is called. Otherwise false is returned.
     */
    virtual bool setTechnology( long technology ) override;

    /// Returns the names of the containers in this database
    virtual std::vector< std::string > containers() override;

    /// Returns a pointer to a container object. The user acquires ownership of that object.
    virtual IContainer* containerHandle( const std::string& name ) override;

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
    /// Reference to the session
    UserSession&                            m_session;
    /// Reference to the file catalog
    IFileCatalog&                           m_catalog;
    /// Transaction type (read/update)
    Io::IoFlag                        m_transactionType;
    /// Reference to the database registry
    DatabaseRegistry&                       m_registry;
    /// The database name
    std::string                             m_name;
    /// The database name spacification
    DatabaseSpecification::NameType         m_nameType;
    /// The technology identifier of the database
    long                                    m_technology;
    /// Checks if the technology identifier has been set
    bool                                    m_technologySet;
    /// The underlying database handler
    DatabaseHandler*                        m_databaseHandler;
    /// Current open mode
    Io::IoFlag                              m_openMode;
    /// Flag indicating whether a connection has been already made once
    bool                                    m_alreadyConnected;
    /// Other names used.
    std::string                             m_the_fid;
    std::string                             m_the_pfn;

    /// Checks in the registry if the database handler already exists
    bool checkInRegistry();
  };
}

#endif
