/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STORAGESVC_DBCONNECTION_H
#define STORAGESVC_DBCONNECTION_H

#include <string>

/*
 *   POOL namespace
 */
namespace pool {

  class DbDatabaseObj;
  
  /** @class DbConnection DbConnection.h StorageSvc/DbConnection.h
    *
    * Definition of the DbConnection class. The connection holds
    * data specific to dealing with one type of Database.
    * 
    * @author: M.Frank
    * @version 1.0
    */
  class DbConnection {
  private:
    /// Reference count
    int m_refCount;
    /// Connection type
    int m_type;
    /// Name of this connection
    std::string m_name;
    /// True handle
    DbDatabaseObj* m_handle;
  public:
    /// Constructor with initializing arguments
    DbConnection(int typ, const std::string& nam, DbDatabaseObj* hdl) 
      : m_refCount(0), m_type(typ), m_name(nam), m_handle(hdl)   {              }
    /// Copy Constructor
    DbConnection(const DbConnection& c) 
      : m_refCount(0), m_type(c.m_type), m_name(c.m_name), m_handle(c.m_handle){}
    /// Standard destructor
    ~DbConnection() = default;

    DbConnection& operator= (const DbConnection&) = delete;

    /// Release token: Decrease reference count and eventually delete.
    int release()    {
      int cnt = --m_refCount;
      if ( 0 >= cnt )   {
        delete this;
      }
      return cnt;
    }
    /// Increase reference count 
    int addRef()                              { return ++m_refCount;  }
    /// Access object identifier
    DbDatabaseObj* handle()                   { return m_handle;      }
    const DbDatabaseObj* handle() const       { return m_handle;      }
    /// Access Database identifier
    const std::string& name() const           { return m_name;        }
    /// Access technoliogy type
    int type() const                          { return m_type;        }
  };
}      // namespace pool

#endif /// STORAGESVC_DBCONNECTION_H
