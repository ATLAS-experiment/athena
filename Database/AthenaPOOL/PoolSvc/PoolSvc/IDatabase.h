/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_IDATABASE_H
#define POOLSVC_IDATABASE_H

#include "StorageSvc/pool.h"

// includes
#include <string>
#include <vector>

namespace pool {

  // forward declarations
  class IContainer;
  class ITechnologySpecificAttributes;

  struct DatabaseSpecification {
    /** Enumeration type specifying the database name field,
     * wherever the latter is used in methods accessing a database.
     */
    enum NameType { UNDEFINED,
                    PFN,    /// Physical File Name
                    FID,    /// File IDentifier
                    LFN     /// Logical File Name
    };
  };

  /** @class IDatabase IDatabase.h PoolSvc/IDatabase.h
   *
   *  IDatabase is the base class for database objects
   *
   */

  class IDatabase {
  public:
    /// Empty destructor
    virtual ~IDatabase() {};

    /// Connects explicitly to the database for read operations
    virtual void connectForRead() = 0;

    /// Connects explicitly to the database for write/update operations
    virtual void connectForWrite() = 0;

    /// Disconnects from the database
    virtual void disconnect() = 0;

    /// Returns the opening mode. It can be used to check whether the database is connected.
    virtual Io::IoFlag openMode() const = 0;

    /// Returns the file identifier of this database
    virtual const std::string& fid() = 0;

    /// Returns the physical file name of this database
    virtual const std::string& pfn() = 0;

    /** Sets the technology identifier for this database.
     *  This can only be called for newly created databases
     *  before the connect method is called. Otherwise false is returned.
     */
    virtual bool setTechnology( long technology ) = 0;

    /// Returns the technology identifier for this database
    virtual long technology() const = 0;

    /// Returns the names of the containers in this database
    virtual std::vector< std::string > containers() = 0;

    /// Returns a pointer to a container object. The user acquires ownership of that object.
    virtual IContainer* containerHandle( const std::string& name ) = 0;

    /// Returns the object holding the technology specific attributes
    virtual ITechnologySpecificAttributes& technologySpecificAttributes() = 0;
  };

}

#endif
