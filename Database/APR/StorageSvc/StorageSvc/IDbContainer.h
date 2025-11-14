/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//
//  Package    : pool/StorageSvc (The pool framework)
//
//  @author      M.Frank
//
//====================================================================
#ifndef POOL_IDBCONTAINER_H
#define POOL_IDBCONTAINER_H

// Framework include files
#include "PersistentDataModel/Token.h"
#include "StorageSvc/pool.h"
#include "StorageSvc/Transaction.h"

#include <vector>
#include <cstdint>

/*
 *   POOL namespace declaration
 */
namespace pool    {

  // Forward declarations
  class DbTypeInfo;
  class DbDatabase;
  class DbContainer;
  class DbOption;
  class DbSelect;

  typedef const class Shape        *ShapeH;

  /** @class IDbContainer IDbContainer.h StorageSvc/IDbContainer.h 
    *
    * Description:
    *
    * Interface to the implementation specific part of a container object
    * objects.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class IDbContainer    {
  protected:
    /// Destructor (called only by sub-classes)
    virtual ~IDbContainer()   {     }

  public:
    /// Release the technology specific implementation
    virtual void release() = 0;
    /// Access to container size
    virtual uint64_t size() = 0;
    /// Get container name
    virtual std::string name() const = 0;
    /// Set options
    virtual DbStatus setOption(const DbOption& refOpt) = 0;
    /// Access options
    virtual DbStatus getOption(DbOption& refOpt) = 0;
    /// Store object in location
    virtual DbStatus store(const void* object,
                           DbContainer&  cntH,
                           ShapeH shape) = 0;
    /// In place allocation of object location
    virtual DbStatus allocate(DbContainer& cntH,
                              const void* object,
                              ShapeH shape,
                              Token::OID_t& oid) = 0;

    /// Number of next record in the container (=size if no delete is allowed)
    virtual uint64_t nextRecordId() = 0;
    /// Suggest next Record ID for tbe next object written - used only with synced indexes
    virtual void useNextRecordId(uint64_t) = 0;

    /// Close the container
    virtual DbStatus close() = 0;
    /// Open the container
    virtual DbStatus open(  DbDatabase&        dbH, 
                            const std::string& nam, 
                            const DbTypeInfo* info, 
                            DbAccessMode mode) = 0;
    /// Check if we can access the container for reading with the given type
    virtual DbStatus checkAccess(DbDatabase&        dbH,
                                 const std::string& nam) const = 0;
    /// Define selection
    virtual DbStatus select(DbSelect& sel) = 0;
    /// Fetch next object address of the selection to set token
    virtual DbStatus fetch(DbSelect& sel) = 0;

    /// Find object within the container and load it into memory
    /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
      * @param  shape     [IN]   Object type
      * @param  linkH     [IN]   Preferred object OID
      * @param  oid      [OUT]   Actual object OID
      * @param  any_next  [IN]   On selection, objects may be skipped.
      *                          If objects are skipped, the actual oid
      *                          will differ from the preferred oid.
      * @return Status code indicating success or failure.
      */
    virtual DbStatus load( void** ptr, ShapeH shape,
                           const Token::OID_t& lnkH,
                           Token::OID_t&       oid,
                           bool                any_next=false) = 0;

    /// Execute Transaction Action
    virtual DbStatus transAct(Transaction::Action) = 0;
  };
}      // End namespace pool
#endif // POOL_IDBCONTAINER_H
