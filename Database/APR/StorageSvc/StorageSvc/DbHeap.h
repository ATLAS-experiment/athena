/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbHeap class definitions
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_STORAGESVC_DBHEAP_H
#define POOL_STORAGESVC_DBHEAP_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/pool.h"

/*
 *  POOL namespace declaration
 */
namespace pool  {

  // Forward declarations
  class DbContainer;
  template <class T> class DbObjectHandle;

  class DbHeap  {
  public:
    /// Standard Constructor
    DbHeap();
    /// Standard Destructor
    virtual ~DbHeap();
    /// Access the object guard size
    static size_t guardSize();
    /// Access the offset of an object
    static Token::OID_t& oid(DbObject* pObj);
    static const Token::OID_t& oid(const DbObject* pObj);
    /// Access the container of an object
    static DbContainer& container(DbObject* pObj);
    static const DbContainer& container(const DbObject* pObj);
    /// Free an object, which was initially allocated from the heap
    static DbStatus free(void* ptr, DbContainer* cntH);
    /// Free an object on the heap
    static DbObject* allocate(size_t siz,
                              DbContainer* cntH,
                              const Token::OID_t* linkH,
                              DbObjectHandle<DbObject>* objH);
  };
}       // End namespace pool
#endif  // POOL_STORAGESVC_DBHEAP_H
