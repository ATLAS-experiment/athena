/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_DATABASE_H
#define POOLSVC_DATABASE_H

#include "AthenaBaseComps/AthMessaging.h"
#include "DataModelRoot/RootType.h"
#include "GaudiKernel/IFileMgr.h"

namespace Gaudi {
  class IFileCatalog;
}
namespace pool {
  class FileDescriptor;
}

class SessionTool;
class DatabaseHandler;
class DatabaseRegistry;
class Container;
class Token;

/** @class Database
 */
class Database final : public AthMessaging
{
public:
  struct Specification {
    /** Enumeration type specifying the database name field,
     * wherever the latter is used in methods accessing a database.
     */
    enum NameType { UNDEFINED,
                    PFN,    /// Physical File Name
                    FID,    /// File IDentifier
                    LFN     /// Logical File Name
    };
  };

  /// Constructor
  Database( SessionTool& session,
            const std::string& name,
            const Specification::NameType nameType );

  /// Destructor
  ~Database();

  /// Connects explicitly to the database for read operations
  void connectForRead();

  /// Reads an object given a token
  void* readObject( const Token& token, void* object = 0 );

  /// Connects explicitly to the database for write/update operations
  void connectForWrite();

  /// Writes an object and returns a token
  Token* writeObject( const std::string& containerName,
                      long minorTechnology,
                      const void* object,
                      const RootType& type );

  /// Disconnects from the database
  void disconnect();

  /// Returns the opening mode. It can be used to check whether the database is connected.
  Io::IoFlag openMode() const;

  /// Returns the file identifier of this database
  const std::string& fid();

  /// Returns the physical file name of this database
  const std::string& pfn();

  /** Sets the technology identifier for this database.
   *  This can only be called for newly created databases
   *  before the connect method is called. Otherwise false is returned.
   */
  bool setTechnology( long technology );

  /// Returns the names of the containers in this database
  std::vector< std::string > containers();

  /// Returns a pointer to a container object. The user acquires ownership of that object.
  Container* containerHandle( const std::string& name );

  // expose FileDescriptor object for the technology specific DB implementation
  pool::FileDescriptor* fileDescriptor();

  bool attributeOfType( const std::string& attributeName,
                        void* data,
                        const std::type_info& typeInfo,
                        const std::string& option );

  bool setAttributeOfType( const std::string& attributeName,
                           const void* data,
                           const std::type_info& typeInfo,
                           const std::string& option );
private:
  /// Reference to the session
  SessionTool&                            m_session;
  /// Reference to the file catalog
  Gaudi::IFileCatalog&                    m_catalog;
  /// Transaction type (read/update)
  Io::IoFlag                              m_transactionType;
  /// Reference to the database registry
  DatabaseRegistry&                       m_registry;
  /// The database name
  std::string                             m_name;
  /// The database name spacification
  Specification::NameType                 m_nameType;
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

#endif
