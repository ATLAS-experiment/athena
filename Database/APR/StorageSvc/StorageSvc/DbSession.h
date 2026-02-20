/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbSession class definition
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//
//  Description: Definition of a Database session and related classes
//
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBSESSION_H
#define POOL_DBSESSION_H

// Framework include files
#include <GaudiKernel/StatusCode.h>
#include "StorageSvc/DbHandleBase.h"

class StatusCode;

/*
 *   POOL namespace declaration
 */
namespace pool  {

  // Forward declarations
  class DbType;
  class DbDomainObj;
  class DbSessionObj;
  class IOODatabase;

  /** Db objects: class DbSession

      Description:
      Handle managing a DbSessionObj, which allows to access
      domains and Databasees of a given storage type.

      @author  M.Frank
      @version 1.0
  */
  class DbSession : public DbHandleBase<DbSessionObj> {
    /// Friend declarations
    friend class DbSessionObj;
  private:
    /// Assign transient object properly (including reference counting)
    void switchPtr(DbSessionObj* obj);
  public:
    /// Constructor
    DbSession();
    /// Copy constructor
    DbSession(const DbSession& copy);
    /// Object constructor
    DbSession(DbSessionObj* session);
    /// Standard destructor
    virtual ~DbSession();
    /// Assignment operator   
    DbSession& operator=(const DbSession& copy)    {
      if ( &copy != this )  {
        switchPtr(copy.m_ptr);
      }
      return *this;
    }
    /// Assignment operator
    DbSession& operator=(const int /* null_obj */ )    {
      switchPtr(0);
      return *this;
    }

    // Move
    DbSession (DbSession&& cp)
    {
      setPtr (cp.m_ptr);
      setType (cp.m_type);
      cp.setPtr (nullptr);
      cp.setType(DbType(0));
    }
    DbSession& operator= (DbSession&& cp)
    {
      if (&cp != this) {
        switchPtr (nullptr);
        setPtr (cp.m_ptr);
        setType (cp.m_type);
        cp.setPtr (nullptr);
        cp.setType(DbType(0));
      }
      return *this;
    }

    /// Access reference counter
    int refCount() const;
    /// Find domain object in session (by technology type)
    DbDomainObj* find(const DbType& type);
    /// Add domain to session
    StatusCode add(DbDomainObj* dom);
    /// Find domain in session
    StatusCode remove(const DbDomainObj* dom);
    /// Open the session in a given mode
    StatusCode open();
    /// Close the session
    StatusCode close();
    /// Allow access to the Database implementation
    IOODatabase* db(const DbType& typ);
  };
}       // End namespace pool
#endif  // POOL_DBSESSION_H
