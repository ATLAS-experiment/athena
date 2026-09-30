/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_DATABASEHANDLER_H
#define POOLSVC_DATABASEHANDLER_H

#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include <vector>
#include <set>
#include <utility>

class Guid;
class Token;

namespace pool {

  // forward declarations
  class IStorageSvc;
  class IContainer;

  /** @class DatabaseHandler
   * 
   * DatabaseHandler is a class taking care of the micro-connections
   * and the micro transactions for a given database.
   * It also gives access to the underlying containers.
   */

  class DatabaseHandler {
  public:
    /// Constructor. Connects to the database
    DatabaseHandler( IStorageSvc& storageSvc,
                     long technology,
                     const std::string& fid,
                     const std::string& pfn,
                     Io::IoFlag accessmode );

    /// Destructor. Disconnects from the database
    ~DatabaseHandler();

    /// Commits the transaction
    bool commitTransaction();

    /// Commits and holds the transaction
    bool commitAndHoldTransaction();

    /// Disconnects the transaction
    bool disconnectTransaction();

    /// Gives the list of containers
    std::vector< std::string > containers();

    /// Returns a container handle
    IContainer* container( const std::string& containerName );

    /// Returns the physical file name
    const std::string& pfn() const;

    /// Returns the file identifier
    const std::string& fid() const;

    /// Returns the technology identifier
    long technology() const;

    /// Returns the access mode
    Io::IoFlag accessMode() const;

    // expose FileDescriptor object for the technology specific DB implementation
    FileDescriptor& fileDescriptor() { return m_fileDescriptor; }

    /// Writes an object and returns a token
    Token* writeObject( const std::string& containerName,
                        long minorTechnology,
                        const void* object,
                        const RootType& type );

    /// Reads an object given a token
    void* readObject( const Token& token, void* object = 0 );

    /// Returns an attribute
    bool attribute( const std::string& attributeName,
                    void* data,
                    const std::type_info& typeInfo,
                    const std::string& option );

    /// Sets an attrtibute
    bool setAttribute( const std::string& attributeName,
                       const void* data,
                       const std::type_info& typeInfo,
                       const std::string& option );

  private:
    /// IStorageSvc reference
    IStorageSvc&      m_storageSvc;
    /// File descriptor for this database
    FileDescriptor    m_fileDescriptor;
    /// Technology identifier
    long              m_technology;
    /// Current access mode
    Io::IoFlag        m_accessMode;
  };
}

#endif
