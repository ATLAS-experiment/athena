/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbObject definition
//--------------------------------------------------------------------
//
//  Package    : (The POOL project)
//
//  Description: Generic handle to persistable objects
//
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBOBJECT_H
#define POOL_DBOBJECT_H 1

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/pool.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbHandleBase.h"
#include "StorageSvc/DbHeap.h"
#include "StorageSvc/DbDatabase.h"
#include "CxxUtils/checker_macros.h"

namespace pool {

  class DbContainer;

  /** @class DbObjectHandle DbObject.h StorageSvc/DbObject.h
    *
    * Description:
    * Base class for object handles.
    *
    * @author  M.Frank
    * @version 1.0
    */
  template <class USER> class DbObjectHandle : public DbHandleBase<USER> {
  private:
    using Base = DbHandleBase<USER>;
    /// Pointer conversion
    const USER* makePtr(void* p)                {  return (const USER*)(p); }
  protected:
    /// Attach object to handle with type: may be accessed by sub-class only
    template<typename T> void _set(const T* ptr, const DbType& typ) {
      _setObject(ptr);
      _setType(typ);
    }
  public:
    using Base::ptr;
    using Base::type;

    /// Set handle type
    void _setType(const DbType& type)     { Base::setType(type);       }
    /// Set object value
    template<class T> void _setObject(T* p)  {
      if ( 0 == ptr() && 0 == p ) return;
      Base::m_ptr =  p;
    }
    void _setObject(const int /* null_obj */ )  { 
      if ( 0 == ptr() ) return;
      Base::m_ptr = 0;
    }
  public:
    /// Standard destructor
    virtual ~DbObjectHandle()                 { _setObject(0);              }
    /// Standard Constructor
    DbObjectHandle()                          {                             }
    /// Constructor with storage type
    DbObjectHandle(const DbType& typ)         { _setType(typ);              }
    /// Constructor with object pointer
    DbObjectHandle(USER* p)                   { _setObject(p);              }

    /// Copy constructor
    DbObjectHandle(const DbObjectHandle<USER>& c) : DbHandleBase<USER>()
    { _set(c.ptr(),c.type());                                               }

    /// Generic assignment operator from base class
    operator const USER*() const              { return ptr();               }
    operator USER*()                          { return ptr();               }

    /// Generic assignment operator
    DbObjectHandle<USER>& operator=(const int /* nuller */)
    { _setObject(0); return *this;                                          }

    DbObjectHandle<USER>& operator=(DbObjectHandle& c) {
      if (&c != this)  _set (c.ptr(), c.type());
      return *this;
    }

    /// Generic assignment operator
    DbObjectHandle<USER>& operator=(USER* obj) {
      if( ptr() != obj ) _setObject(obj);
      return *this;
    }

    /// Equality operator
    template <typename T> bool operator==(const DbHandleBase<T>& objH) const
    { return type() == objH.type() && ptr() == objH.ptr();      }

    /// Retrieve hosting container
    const DbContainer& containedIn() const
    {  return DbHeap::container(ptr());  }

    /// Access object oid
    const Token::OID_t& oid() const
    { return DbHeap::oid(ptr()); }

    /// Access object oid
    Token::OID_t& oid()
    { return DbHeap::oid(ptr()); }

    /// Add persistent association entry
    DbStatus makeLink ATLAS_NOT_THREAD_SAFE (Token* pToken, Token::OID_t& linkH) const  {
      if( pToken )   {
        DbDatabase& dbH = containedIn().containedIn();
        return dbH.makeLink(pToken, linkH);
      }
      return Error;
    }
  };


  template <class USER> using DbHandle = DbObjectHandle< USER>;

}  // End namespace pool

#endif  // POOL_DBOBJECT_H
